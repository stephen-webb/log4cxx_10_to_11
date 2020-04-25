#include "CppFile.h"
#include <fstream>
#include <string>
#include <ctype.h>

#ifdef _MSC_VER
#pragma warning (disable : 4996) // sprintf is used in boost::wave
#endif

///////////////////////////////////////////////////////////////////////////////
//  Include Wave itself
#include <boost/wave.hpp>

///////////////////////////////////////////////////////////////////////////////
// Include the lexer stuff
#include <boost/wave/util/flex_string.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>

// The following file needs to be included only once throughout the whole program.
#include <boost/wave/cpplexer/re2clex/cpp_re2c_lexer.hpp>

    static log4cxx::LoggerPtr
log_s(log4cxx::Logger::getLogger("CppFile"));

/// Processing hooks that enable single file processing
class CppFile::CustomDirectivesHooks
    : public boost::wave::context_policies::default_preprocessing_hooks
{
public: // Hooked methods
    /// Prevent include processing
    template <typename ContextT>
    bool found_include_directive
        ( ContextT const&    ctx
        , std::string const& filename
        , bool               include_next
        )
    {
        LOG4CXX_TRACE(log_s, "include " << filename);
        return true;    // skip all #includes
    }
    /// Prevent directive processing
    template <typename ContextT, typename TokenT>
    bool found_directive(ContextT const &ctx, TokenT const &directive)
    {
        LOG4CXX_TRACE(log_s, "directive " << directive.get_value());
        return true;    // skip all directives
    }
};

/// Boost Wave types
typedef boost::wave::util::file_position<BOOST_WAVE_STRINGTYPE> position_type;
typedef boost::wave::cpplexer::lex_token<position_type> TokenType;
typedef boost::wave::cpplexer::lex_iterator<TokenType> lex_iterator_type;
typedef boost::wave::context
    < std::string::iterator
    , lex_iterator_type
    , boost::wave::iteration_context_policies::load_file_to_string
    , CppFile::CustomDirectivesHooks
    > ContextType;

/// Enables output in C-style escaped characters of a string
template <class StringType>
struct CStringRef
{
    StringType const& value;
    CStringRef(StringType const& value_)
        : value(value_)
        {}
};

/// Put C-style escaped characters of \c item onto \c stream
template <class StringType>
    std::ostream&
operator<<(std::ostream& stream, CStringRef<StringType> const& item)
{
    for (std::size_t i = 0; i < item.value.size(); ++i)
    {
        switch (item.value[i])
        {
        case '\r':  stream << "\\r"; break;
        case '\n':  stream << "\\n"; break;
        case '\t':  stream << "\\t"; break;
        default:
            if (isprint(item.value[i]))
                stream << item.value[i];
            else
                stream << "0x" << std::hex << int(item.value[i]);
            break;
        }
    }
    return stream;
}

/// Put attributes pointed to by \c pItem onto \c stream
    std::ostream&
operator<<(std::ostream& stream, ContextType::iterator_type const& pItem)
{
    boost::wave::token_id id = *pItem;
    stream << boost::wave::get_token_name(id)
        << "(#" << BASEID_FROM_TOKEN(id) << ')';

    stream << ": >" << CStringRef<TokenType::string_type>(pItem->get_value()) << '<';
    stream << " (" << pItem->get_position().get_line()
        << ',' << pItem->get_position().get_column()
        << ')';
    return stream;
}

///////////////////////////////////////////////////////////////////////////////
//  CppFile implementation

// Put \c item onto \c os
    std::ostream&
operator<<(std::ostream& stream, CppFile::PositionType const& item)
{
    stream << item.line << ',' << item.column;
    return stream;
}

/// The index into \c m_content corresponding to (1-based) index.line and index.col
    size_t
CppFile::GetContentIndex(const PositionType& index) const
{
    size_t result;
    if (0 < index.line && index.line <= m_lineIndex.size())
        result = m_lineIndex[index.line - 1] + index.column - 1;
    else
        result = m_content.size();
    LOG4CXX_TRACE(log_s, "GetContentIndex: " << index << " result " << result);
    return result;
}

/// The number of instances of the identifier \c name
    size_t
CppFile::GetIdentifierCount(const StringType& name) const
{
    size_t result = 0;
    StringPositionMap::const_iterator pItem = m_identiferPositions.find(name);
    if (m_identiferPositions.end() != pItem)
        result = pItem->second.size();
    return result;
}

/// The number of function call style usages of the identifier \c name
    size_t
CppFile::GetFunctionCount(const StringType& name) const
{
    size_t result = 0;
    StringPositionMap::const_iterator pItem = m_identiferPositions.find(name);
    if (m_identiferPositions.end() == pItem)
        ;
    else for (size_t i = 0; i < pItem->second.size(); ++i)
    {
        boost::wave::token_id tokenId = GetNonWhitespaceTokenAfter(pItem->second[i]);
        if (boost::wave::T_LEFTPAREN == tokenId)
            ++result;
    }
    return result;
}

/// Has an been made between \c start and before \c end
    bool
CppFile::HasUpdateBetween(const PositionType& start, const PositionType& end) const
{
    static const int MaxUpdatesPerPosition = 100;
    UpdateKey keyStart(start, -MaxUpdatesPerPosition);
    UpdateKey keyEnd(end, MaxUpdatesPerPosition);
    UpdateMap::const_iterator pUpdate = m_updates.lower_bound(keyStart);
    bool found = m_updates.end() != pUpdate && pUpdate->first < keyEnd;
    LOG4CXX_TRACE(log_s, "HasUpdateBetween: " << start << " and " << end << " found? " << found);
    return found;
}

/// The id (and optionally position) of the first compiler token before \c index
    boost::wave::token_id
CppFile::GetNonWhitespaceTokenBefore(const PositionType& index, PositionType* resultIndex) const
{
    boost::wave::token_id result = boost::wave::T_EOI;
    IndexedToken::const_iterator pItem = m_tokenPositions.lower_bound(index);
    while (m_tokenPositions.end() != pItem && (pItem->first == index
        || IS_CATEGORY(pItem->second, boost::wave::WhiteSpaceTokenType)
        || IS_CATEGORY(pItem->second, boost::wave::EOLTokenType) ))
      --pItem;
    if (m_tokenPositions.end() != pItem)
    {
        result = pItem->second;
        LOG4CXX_TRACE(log_s, "GetNonWhitespaceTokenBefore: " << index
            << " token " << boost::wave::get_token_name(result)
            << " at " << pItem->first
            );
        if (resultIndex)
            *resultIndex = pItem->first;
    }
    return result;
}

/// The id (and optionally position) of the first compiler token before the parenthesis matching \c index
    boost::wave::token_id
CppFile::GetNonWhitespaceTokenBeforeOtherParen(const PositionType& index, PositionType* resultIndex) const
{
    LOG4CXX_TRACE(log_s, "GetNonWhitespaceTokenBeforeOtherParen: " << index);
    boost::wave::token_id result = boost::wave::T_EOI;
    IndexMap::const_iterator pMate = m_parenMate.find(index);
    if (m_parenMate.end() != pMate)
        result = GetNonWhitespaceTokenBefore(pMate->second, resultIndex);
    return result;
}

/// The id (and optionally position) of the first compiler token after \c index
    boost::wave::token_id
CppFile::GetNonWhitespaceTokenAfter(const PositionType& index, PositionType* resultIndex) const
{
    boost::wave::token_id result = boost::wave::T_EOI;
    IndexedToken::const_iterator pItem = m_tokenPositions.upper_bound(index);
    while (m_tokenPositions.end() != pItem && (pItem->first == index
        || IS_CATEGORY(pItem->second, boost::wave::WhiteSpaceTokenType)
        || IS_CATEGORY(pItem->second, boost::wave::EOLTokenType) ))
      ++pItem;
    if (m_tokenPositions.end() != pItem)
    {
        LOG4CXX_TRACE(log_s, "GetNonWhitespaceTokenAfter: " << index
            << " token " << boost::wave::get_token_name(pItem->second)
            << " at " << pItem->first
            );
        if (resultIndex)
            *resultIndex = pItem->first;
        result = pItem->second;
    }
    return result;
}

/// Has this been loaded?
    bool
CppFile::IsValid() const
{
    return m_lineIndex.size() - 1 <= m_processed.line;
}

/// Load \c read into various indexing attributes
    bool
CppFile::LoadFile(const PathType& path)
{
    LOG4CXX_DEBUG(log_s, "LoadFile: " << path);
    std::ifstream instream(path.c_str());
    if (!instream.is_open())
        return false;
    instream.unsetf(std::ios::skipws);
    bool ok = false;
    position_type current_position;
    try
    {
        m_identiferPositions.clear();
        m_parenMate.clear();
        m_tokenPositions.clear();
        m_content = std::string
            ( std::istreambuf_iterator<char>(instream.rdbuf())
            , std::istreambuf_iterator<char>()
            );
        SetLineIndex();
        CustomDirectivesHooks hooks;
        ContextType ctx(m_content.begin(), m_content.end(), path.string().c_str(), hooks);
        ctx.set_language(boost::wave::enable_preserve_comments(ctx.get_language()));
        ContextType::iterator_type first = ctx.begin();
        ContextType::iterator_type last = ctx.end();
        std::vector<PositionType> parenStack;
        while (first != last)
        {
            LOG4CXX_TRACE(log_s, first);
            boost::wave::token_id tokenId = *first;
            current_position = first->get_position();
            m_processed = PositionType{current_position.get_line(), current_position.get_column()};
            if (boost::wave::T_LEFTPAREN == tokenId)
                parenStack.push_back(m_processed);
            else if (boost::wave::T_RIGHTPAREN == tokenId && !parenStack.empty())
            {
                LOG4CXX_TRACE(log_s, "LeftParen " << parenStack.back());
                m_parenMate[m_processed] = parenStack.back();
                m_parenMate[parenStack.back()] = m_processed;
                parenStack.pop_back();
            }
            else if (boost::wave::T_IDENTIFIER == tokenId)
            {
                StringType identifier = first->get_value().c_str();
                m_identiferPositions[identifier].push_back(m_processed);
            }
            else if (boost::wave::T_UNKNOWN == tokenId)
            {
                LOG4CXX_WARN(log_s, "Unknown token (" << CStringRef<BOOST_WAVE_STRINGTYPE>(first->get_value()) << ')'
                    << " at " << path
                    << '(' << current_position.get_line()
                    << ',' << current_position.get_column() << ')'
                    );
            }
            if (boost::wave::T_CCOMMENT == tokenId)
                m_processed.line += boost::wave::context_policies::util::ccomment_count_newlines(*first);
            m_tokenPositions[m_processed] = tokenId;
            ++first;
        }
        ok = true;
    }
    catch (boost::wave::cpplexer::lexing_exception const& e)
    {
        LOG4CXX_WARN(log_s, e.description()
            << " at " << e.file_name()
            << '(' << e.line_no() << ')'
            );
    }
    catch (boost::wave::cpp_exception const& e)
    {
        LOG4CXX_WARN(log_s, e.description()
            << " at " << e.file_name()
            << '(' << e.line_no() << ')'
            );
    }
    catch (std::exception const& e)
    {
        LOG4CXX_WARN(log_s, e.what()
            << " at " << current_position.get_file()
            << '(' << current_position.get_line() << ')'
            );
    }
    catch (...)
    {
        LOG4CXX_WARN(log_s, "unexpected exception caught"
            << " at " << current_position.get_file()
            << '(' << current_position.get_line() << ')'
            );
    }
    return ok;
}

/// Append \c text after \c lineCol
    void
CppFile::AppendText(const PositionType& lineCol, const StringType& text)
{
    LOG4CXX_DEBUG(log_s, "AppendText: " << CStringRef<StringType>(text) << " at " << lineCol);
    size_t contentIndex = GetContentIndex(lineCol) + 1;
    UpdateData newText = {contentIndex, Insert, text, contentIndex};
    UpdateKey key(lineCol, 0);
    while (0 < m_updates.count(key))
        ++key.second;
    m_updates[key] = newText;
}

/// Insert \c text before \c lineCol
    void
CppFile::InsertText(const PositionType& lineCol, const StringType& text)
{
    LOG4CXX_DEBUG(log_s, "InsertText: " << CStringRef<StringType>(text) << " at " << lineCol);
    size_t contentIndex = GetContentIndex(lineCol);
    UpdateData newText = {contentIndex, Insert, text, contentIndex};
    UpdateKey key(lineCol, 0);
    while (0 < m_updates.count(key))
        --key.second;
    m_updates[key] = newText;
}

/// Initialize m_lineIndex
    void
CppFile::SetLineIndex()
{
    m_lineIndex.clear();
    m_lineIndex.push_back(0);
    for (;;)
    {
        size_t eol = m_content.find('\n', m_lineIndex.back());
        if (m_content.npos == eol)
            break;
        m_lineIndex.push_back(eol + 1);
    }
    LOG4CXX_DEBUG(log_s, "SetLineIndex: contentSize " << m_content.size() << " lineCount " << m_lineIndex.size());
}


/// Write the (possibly) modified content to \c path
    bool
CppFile::StoreFile(const PathType& path)
{
    LOG4CXX_DEBUG(log_s, "StoreFile: " << path);
    std::ofstream stream(path.c_str());
    Store(stream);
    stream.close();
    return !stream.bad();
}

/// Write the (possibly) modified content to \c os
    void
CppFile::Store(std::ostream& os)
{
    size_t outIndex = 0;
    for (UpdateMap::const_iterator pUpdate = m_updates.begin()
        ; pUpdate != m_updates.end()
        ; ++pUpdate)
    {
        LOG4CXX_TRACE(log_s, "Store: at " << pUpdate->first.first);
        const EditType& editType = pUpdate->second.type;
        size_t copyToIndex = pUpdate->second.at;
        if (outIndex < copyToIndex)
        {
            LOG4CXX_TRACE(log_s, "Store: copy " << outIndex << " to " << copyToIndex);
            os << m_content.substr(outIndex, copyToIndex - outIndex);
        }
        if (Delete != editType)
        {
            LOG4CXX_TRACE(log_s, "Store: insert " << CStringRef<StringType>(pUpdate->second.text));
            os << pUpdate->second.text;
        }
        outIndex = pUpdate->second.resumeAt;
    }
    if (outIndex < m_content.size())
    {
        LOG4CXX_TRACE(log_s, "Store: copy " << outIndex << " to " << m_content.size());
        os << m_content.substr(outIndex);
    }
}

///////////////////////////////////////////////////////////////////////////////
// FunctionIterator implementation

    log4cxx::LoggerPtr
CppFile::FunctionIterator::m_log(log4cxx::Logger::getLogger("FunctionIterator"));

/// An Off() iterator for function call names starting with \c prefix
CppFile::FunctionIterator::FunctionIterator(CppFile& file, const StringType& prefix)
    : m_file(file)
    , m_prefix(prefix)
    , m_identifier(m_file.m_identiferPositions.end())
{}

/// Skip function calls matching \c identifierPrefix
    void
CppFile::FunctionIterator::AddExclusion(const StringType& identifierPrefix)
{
    LOG4CXX_DEBUG(m_log, "AddExclusion: " << identifierPrefix);
    m_exclusions.push_back(identifierPrefix);
}

/// Add a semicolan after the closing parenthesis
    void
CppFile::FunctionIterator::AddSemicolon()
{
    LOG4CXX_DEBUG(m_log, "AddSemicolon: " << m_item.paramEnd);
    PositionType insertPos = { m_item.paramEnd.line, m_item.paramEnd.column + 1};
    m_file.InsertText(insertPos, ";");
}

/// Add an opening before the function and a closing brace after the statement
    void
CppFile::FunctionIterator::InsertBraces()
{
    LOG4CXX_DEBUG(m_log, "InsertBraces: " << m_item.identifier << " to " << m_item.paramEnd);
    PositionType previousToken;
    m_file.GetNonWhitespaceTokenBefore(m_item.identifier, &previousToken);
    StringType indent;
    if (previousToken.line < m_item.identifier.line)
    {
        PositionType startOfPreviousLine = {previousToken.line, 1};
        PositionType firstTokenOfPreviousLine;
        m_file.GetNonWhitespaceTokenAfter(startOfPreviousLine, &firstTokenOfPreviousLine);
        size_t previousIndex[2] =
            { m_file.GetContentIndex(startOfPreviousLine)
            , m_file.GetContentIndex(firstTokenOfPreviousLine)
            };
        indent = m_file.m_content.substr(previousIndex[0], previousIndex[1] - previousIndex[0]);
        PositionType startOfLine = {m_item.identifier.line, 1};
        m_file.InsertText(startOfLine, indent + "{\n");
    }
    else
        m_file.InsertText(m_item.identifier, "{");
    PositionType nextToken;
    m_file.GetNonWhitespaceTokenAfter(m_item.paramEnd, &nextToken);
    if (m_item.identifier.line < nextToken.line)
    {
        PositionType startOfNextLine = {m_item.paramEnd.line + 1, 1};
        m_file.InsertText(startOfNextLine, indent + "}\n");
    }
    else
        m_file.AppendText(m_item.paramEnd, " }");
}

/// Is the next non-white-space token a semicolon or comma? - Precondition: !Off()
    bool
CppFile::FunctionIterator::HasStatementTerminator() const
{
    boost::wave::token_id tokenId = m_file.GetNonWhitespaceTokenAfter(m_item.paramEnd);
    return boost::wave::T_SEMICOLON == tokenId ||
           boost::wave::T_COLON == tokenId ||
           boost::wave::T_COMMA == tokenId;
}

/// Is the previous non-white-space token not in [else, comma, semicolon, brace]? - Precondition: !Off()
    bool
CppFile::FunctionIterator::IsCompoundStatementBody() const
{
    PositionType previousToken;
    boost::wave::token_id tokenId = m_file.GetNonWhitespaceTokenBefore(m_item.identifier, &previousToken);
    if (boost::wave::T_RIGHTPAREN == tokenId)
    {
        boost::wave::token_id statementId = m_file.GetNonWhitespaceTokenBeforeOtherParen(previousToken);
        return boost::wave::T_CATCH == statementId ||
               boost::wave::T_FOR == statementId ||
               boost::wave::T_IF == statementId ||
               boost::wave::T_SWITCH == statementId ||
               boost::wave::T_WHILE == statementId;
    }
    return !m_file.HasUpdateBetween(previousToken, m_item.identifier) &&
           boost::wave::T_ELSE != tokenId &&
           boost::wave::T_LEFTBRACE != tokenId &&
           boost::wave::T_RIGHTBRACE != tokenId &&
           boost::wave::T_COLON != tokenId &&
           boost::wave::T_SEMICOLON != tokenId &&
           boost::wave::T_COMMA != tokenId;
}

/// Move to the next item. Precondition: !Off()
    void
CppFile::FunctionIterator::Forth()
{
    ++m_instance;
    while (!OffInstance())
    {
        if (SetItem())
            return;
        ++m_instance;
    }
    ++m_identifier;
    StartInstance();
}

// Is this iterator beyond the end or before the start?
    bool
CppFile::FunctionIterator::Off() const
{
    return m_file.m_identiferPositions.end() == m_identifier ||
        0 != m_identifier->first.compare(0, m_prefix.size(), m_prefix);
}

// Set \c m_item - Precondition: !OffInstance()
    bool
CppFile::FunctionIterator::SetItem()
{
    m_item.identifier = *m_instance;
    boost::wave::token_id tokenId = m_file.GetNonWhitespaceTokenAfter(*m_instance, &m_item.paramStart);
    if (boost::wave::T_LEFTPAREN != tokenId)
        return false;
    IndexMap::const_iterator closeParen = m_file.m_parenMate.find(m_item.paramStart);
    if (m_file.m_parenMate.end() == closeParen)
        return false;
    m_item.paramEnd = closeParen->second;
    LOG4CXX_DEBUG(m_log, m_identifier->first
        << " at " << m_item.identifier
        << " to " << m_item.paramEnd
        );
    return true;
}

// Move to the first item
    void
CppFile::FunctionIterator::Start()
{
    LOG4CXX_DEBUG(m_log, "Start: " << m_prefix);
    m_identifier = m_file.m_identiferPositions.lower_bound(m_prefix);
    StartInstance();
}

// Move to the first instance of the selected identifier
    void
CppFile::FunctionIterator::StartInstance()
{
    while (!Off())
    {
        LOG4CXX_TRACE(m_log, m_identifier->first);
        if (m_exclusions.end() == std::find_if(m_exclusions.begin(), m_exclusions.end(),
            [this](const StringType& prefix) -> bool
                { return 0 == m_identifier->first.compare(0, prefix.size(), prefix); }
            ))
        {
            m_instance = m_identifier->second.begin();
            m_instanceEnd = m_identifier->second.end();
            while (!OffInstance())
            {
                if (SetItem())
                    return;
                ++m_instance;
            }
        }
        ++m_identifier;
    }
}
