// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "AbstractRow.h"                            // Embeds AbstractRow.
#include "BitFunnel/NonCopyable.h"                  // Inherits from NonCopyable.
#include "BitFunnel/Utilities/IPersistableObject.h" // Inherits from IPersistableObject.


namespace BitFunnel
{
    class ICodeGenerator;
    class IObjectParser;


    class CompileNode : public IPersistableObject,
                        protected NonCopyable
    {
    public:
        class Binary;
        class Unary;

        // RankDown are "normal" nodes. AndRowJz ands something into an
        // accumulator.
        //
        // RankZero nodes are for things that can't be done as vectored
        // calculations -- they produce a single bit. AndTree gives you a single
        // bit.

        // RankDown nodes
        class AndRowJz;
        class LoadRowJz;
        class Or;
        class RankDown;
        class Report;

        // RankZero nodes
        class AndTree;
        class LoadRow;
        class Not;
        class OrTree;

        enum NodeType
        {
            Invalid = -2,
            Null = -1,

            // DESIGN NOTE: legal node types have consecutive values starting
            // at zero so that TypeCount is equal to the number of legal nodes.

            // RankDown operations.
            opAndRowJz = 0,
            opLoadRowJz,
            opOr,
            opRankDown,
            opReport,

            // RankZero operations
            opAndTree,
            opLoadRow,
            opNot,
            opOrTree,

            // Total number of node types.
            TypeCount
        };

        virtual NodeType GetType() const = 0;

        virtual void Compile(ICodeGenerator& codeGenerator) const = 0;

        //
        // IPersistableObject methods
        //
        int GetTypeTag() const;
        const char* GetTypeName() const;

        //
        // Static methods
        //
        static int GetType(const char*);


        //Static parsing methods.
        static CompileNode const & Parse(IObjectParser& parser);
        static CompileNode const * ParseNullable(IObjectParser& parser);
    };


    class CompileNode::Binary : public CompileNode
    {
    public:
        Binary(CompileNode const & left, CompileNode const & right);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        CompileNode const & GetLeft() const;
        CompileNode const & GetRight() const;

    protected:
        static char const * c_childrenFieldName;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        CompileNode const & m_left;
        CompileNode const & m_right;
    };


    class CompileNode::AndRowJz : public CompileNode
    {
    public:
        AndRowJz(AbstractRow const & row, CompileNode const & child);
        AndRowJz(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        AbstractRow const & GetRow() const;
        CompileNode const & GetChild() const;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        AbstractRow const m_row;
        CompileNode const & m_child;

        static char const * c_rowFieldName;
        static char const * c_childFieldName;
    };


    class CompileNode::LoadRowJz : public CompileNode
    {
    public:
        LoadRowJz(AbstractRow const & row, CompileNode const & child);
        LoadRowJz(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        AbstractRow const & GetRow() const;
        CompileNode const & GetChild() const;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        AbstractRow const m_row;
        CompileNode const & m_child;

        static char const * c_rowFieldName;
        static char const * c_childFieldName;
    };


    class CompileNode::Or : public CompileNode::Binary
    {
    public:
        Or(CompileNode const & left, CompileNode const & right);

        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        static Or const & Parse(IObjectParser& parser);
    };


    class CompileNode::RankDown : public CompileNode
    {
    public:
        RankDown(Rank delta, CompileNode const & child);
        RankDown(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        Rank GetDelta() const;
        CompileNode const & GetChild() const;

    private:
        // WARNING: The persistence format depends on the order in which the
        // following two members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the constructor and
        // and the Format() method.
        Rank m_delta;
        CompileNode const & m_child;

        static char const * c_deltaFieldName;
        static char const * c_childFieldName;
    };


    class CompileNode::Report : public CompileNode
    {
    public:
        Report(CompileNode const * child);
        Report(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        CompileNode const * GetChild() const;

    private:
        CompileNode const * m_child;

        static char const * c_childFieldName;
    };


    class CompileNode::AndTree : public CompileNode::Binary
    {
    public:
        AndTree(CompileNode const & left, CompileNode const & right);

        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        static AndTree const & Parse(IObjectParser& parser);
    };


    class CompileNode::OrTree : public CompileNode::Binary
    {
    public:
        OrTree(CompileNode const & left, CompileNode const & right);

        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        static OrTree const & Parse(IObjectParser& parser);
    };


    class CompileNode::LoadRow : public CompileNode
    {
    public:
        LoadRow(AbstractRow const & row);
        LoadRow(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        AbstractRow const & GetRow() const;

    private:
        AbstractRow const m_row;

        static char const * c_rowFieldName;
    };


    class CompileNode::Not : public CompileNode
    {
    public:
        Not(CompileNode const & child);
        Not(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;
        void Compile(ICodeGenerator& codeGenerator) const;

        NodeType GetType() const;

        CompileNode const & GetChild() const;

    private:
        CompileNode const & m_child;

        static char const * c_childFieldName;
    };
}
