// file      : libstudxml/parser.hxx -*- C++ -*-
// copyright : Copyright (c) 2013-2019 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef LIBSTUDXML_PARSER_HXX
#define LIBSTUDXML_PARSER_HXX

#include "details/pre.hxx"
//
#include <map>
#include <vector>
#include <string>
#include <iosfwd>
#include <cstddef> // std::size_t

#include "details/config.hxx" // STUDXML_NOTHROW_NOEXCEPT,
                              // LIBSTUDXML_EXTERNAL_EXPAT

#ifndef LIBSTUDXML_EXTERNAL_EXPAT
#  include "details/expat/expat.h"
#else
#  include "details/expat/expat.h"
#endif

// We only support UTF-8 Expat.
//
#ifdef XML_UNICODE
#  error UTF-16 expat (XML_UNICODE defined) is not supported
#endif

#include "forward.hxx"
#include "qname.hxx"
#include "content.hxx"
#include "exception.hxx"

#include "details/export.hxx"

namespace xml
{
  class parsing: public exception
  {
  public:
    virtual
    ~parsing () STUDXML_NOTHROW_NOEXCEPT {}

    parsing (const std::string& name,
             unsigned long long line,
             unsigned long long column,
             const std::string& description);

    parsing (const parser& p, const std::string& description);

    const std::string&
    name () const {return name_;}

    unsigned long long
    line () const {return line_;}

    unsigned long long
    column () const {return column_;}

    const std::string&
    description () const {return description_;}

    virtual const char*
    what () const STUDXML_NOTHROW_NOEXCEPT {return what_.c_str ();}

  private:
    LIBSTUDXML_EXPORT void
    init ();

  private:
    std::string name_;
    unsigned long long line_;
    unsigned long long column_;
    std::string description_;
    std::string what_;
  };

  class LIBSTUDXML_EXPORT parser
  {
  public:
    typedef xml::qname qname_type;
    typedef xml::content content_type;

    typedef unsigned short feature_type;

    // If both receive_attributes_event and receive_attributes_map are
    // specified, then receive_attributes_event is assumed.
    //
    static const feature_type receive_elements = 0x0001;
    static const feature_type receive_characters = 0x0002;
    static const feature_type receive_attributes_map = 0x0004;
    static const feature_type receive_attributes_event = 0x0008;
    static const feature_type receive_namespace_decls = 0x0010;

    static const feature_type receive_default = receive_elements |
                                                receive_characters |
                                                receive_attributes_map;

    // Parse std::istream. Input name is used in diagnostics to identify
    // the document being parsed.
    //
    // If stream exceptions are enabled then std::ios_base::failure
    // exception is used to report io errors (badbit and failbit).
    // Otherwise, those are reported as the parsing exception.
    //
    parser (std::istream&,
            const std::string& input_name,
            feature_type = receive_default);

    // Parse memory buffer that contains the whole document. Input name
    // is used in diagnostics to identify the document being parsed.
    //
    parser (const void* data,
            std::size_t size,
            const std::string& input_name,
            feature_type = receive_default);

    const std::string&
    input_name () const {return iname_;}

    ~parser ();

  private:
    parser (const parser&);
    parser& operator= (const parser&);

    // Parsing events.
    //
  public:
    enum event_type
    {
      // If adding new events, also update the stream insertion operator.
      //
      start_element,
      end_element,
      start_attribute,
      end_attribute,
      characters,
      start_namespace_decl,
      end_namespace_decl,
      eof
    };

    event_type
    next ();

    // Get the next event and make sure that it's what's expected. If it
    // is not, then throw an appropriate parsing exception.
    //
    void
    next_expect (event_type);

    void
    next_expect (event_type, const std::string& name);

    void
    next_expect (event_type, const qname_type& qname);

    void
    next_expect (event_type, const std::string& ns, const std::string& name);

    event_type
    peek ();

    // Return the even that was last returned by the call to next() or
    // peek().
    //
    event_type
    event () {return event_;}

    // Event data.
    //
  public:
    const qname_type& qname () const {return *pqname_;}

    const std::string& namespace_ () const {return pqname_->namespace_ ();}
    const std::string& name () const {return pqname_->name ();}
    const std::string& prefix () const {return pqname_->prefix ();}

    std::string& value () {return *pvalue_;}
    const std::string& value () const {return *pvalue_;}
    template <typename T> T value () const;

    unsigned long long line () const {return line_;}
    unsigned long long column () const {return column_;}

    // Attribute map lookup. If attribute is not found, then the version
    // without the default value throws an appropriate parsing exception
    // while the version with the default value returns that value.
    //
    // Note also that there is no attribute(ns,name) version since it
    // would conflict with attribute(name,dv) (qualified attributes
    // are not very common).
    //
    // Attribute map is valid throughout at the "element level" until
    // end_element and not just during start_element. As a special case,
    // the map is still valid after peek() that returned end_element until
    // this end_element event is retrieved with next().
    //
    const std::string&
    attribute (const std::string& name) const;

    template <typename T>
    T
    attribute (const std::string& name) const;

    std::string
    attribute (const std::string& name,
               const std::string& default_value) const;

    template <typename T>
    T
    attribute (const std::string& name, const T& default_value) const;

    const std::string&
    attribute (const qname_type& qname) const;

    template <typename T>
    T
    attribute (const qname_type& qname) const;

    std::string
    attribute (const qname_type& qname,
               const std::string& default_value) const;

    template <typename T>
    T
    attribute (const qname_type& qname, const T& default_value) const;

    bool
    attribute_present (const std::string& name) const;

    bool
    attribute_present (const qname_type& qname) const;

    // Low-level attribute map access. Note that this API assumes
    // all attributes are handled.
    //
    struct attribute_value_type
    {
      std::string value;
      mutable bool handled;
    };

    typedef std::map<qname_type, attribute_value_type> attribute_map_type;

    const attribute_map_type&
    attribute_map () const;

    // Optional content processing.
    //
  public:
    // Note that you cannot get/set content while peeking.
    //
    void
    content (content_type);

    content_type
    content () const;

    // Versions that also set the content. Event type must be start_element.
    //
    void
    next_expect (event_type, const std::string& name, content_type);

    void
    next_expect (event_type, const qname_type& qname, content_type);

    void
    next_expect (event_type,
                 const std::string& ns, const std::string& name,
                 content_type);

    // Helpers for parsing elements with simple content. The first two
    // functions assume that start_element has already been parsed. The
    // rest parse the complete element, from start to end.
    //
    // Note also that as with attribute(), there is no (namespace,name)
    // overload since it would conflicts with (namespace,default_value).
    //
  public:
    std::string
    element ();

    template <typename T>
    T
    element ();

    std::string
    element (const std::string& name);

    std::string
    element (const qname_type& qname);

    template <typename T>
    T
    element (const std::string& name);

    template <typename T>
    T
    element (const qname_type& qname);

    std::string
    element (const std::string& name, const std::string& default_value);

    std::string
    element (const qname_type& qname, const std::string& default_value);

    template <typename T>
    T
    element (const std::string& name, const T& default_value);

    template <typename T>
    T
    element (const qname_type& qname, const T& default_value);

    // C++11 range-based for support. Generally, the iterator interface
    // doesn't make much sense for the parser so for now we have an
    // implementation that is just enough to the range-based for.
    //
  public:
    struct iterator
    {
      typedef event_type value_type;

      iterator (parser* p = nullptr, event_type e = eof): p_ (p), e_ (e) {}
      value_type operator* () const {return e_;}
      iterator& operator++ () {e_ = p_->next (); return *this;}

      // Comparison only makes sense when comparing to end (eof).
      //
      bool operator== (iterator y) const {return e_ == eof && y.e_ == eof;}
      bool operator!= (iterator y) const {return !(*this == y);}

    private:
      parser* p_;
      event_type e_;
    };

    iterator begin () {return iterator (this, next ());}
    iterator end () {return iterator (this, eof);}

  private:
    static void XMLCALL
    start_element_ (void*, const XML_Char*, const XML_Char**);

    static void XMLCALL
    end_element_ (void*, const XML_Char*);

    static void XMLCALL
    characters_ (void*, const XML_Char*, int);

    static void XMLCALL
    start_namespace_decl_ (void*, const XML_Char*, const XML_Char*);

    static void XMLCALL
    end_namespace_decl_ (void*, const XML_Char*);

  private:
    void
    init ();

    event_type
    next_ (bool peek);

    event_type
    next_body ();

    void
    handle_error ();

  private:
    // If size_ is 0, then data is std::istream. Otherwise, it is a buffer.
    //
    union
    {
      std::istream* is;
      const void* buf;
    } data_;

    std::size_t size_;

    const std::string iname_;
    feature_type feature_;

    XML_Parser p_;
    std::size_t depth_;
    bool accumulate_; // Whether we are accumulating character content.
    enum {state_next, state_peek} state_;
    event_type event_;
    event_type queue_;

    qname_type qname_;
    std::string value_;

    // These are used to avoid copying when we are handling attributes
    // and namespace decls.
    //
    const qname_type* pqname_;
    std::string* pvalue_;

    unsigned long long line_;
    unsigned long long column_;

    // Attributes as events.
    //
    struct attribute_type
    {
      qname_type qname;
      std::string value;
    };

    typedef std::vector<attribute_type> attributes;

    attributes attr_;
    attributes::size_type attr_i_; // Index of the current attribute.

    // Namespace declarations.
    //
    typedef std::vector<qname_type> namespace_decls;

    namespace_decls start_ns_;
    namespace_decls::size_type start_ns_i_; // Index of the current decl.

    namespace_decls end_ns_;
    namespace_decls::size_type end_ns_i_; // Index of the current decl.

    // Element state consisting of the content model and attribute map.
    //
    struct element_entry
    {
      element_entry (std::size_t d, content_type c = content_type::mixed)
          : depth (d), content (c), attr_unhandled_ (0) {}

      std::size_t depth;
      content_type content;
      attribute_map_type attr_map_;
      mutable attribute_map_type::size_type attr_unhandled_;
    };

    typedef std::vector<element_entry> element_state;
    std::vector<element_entry> element_state_;

    // Empty attribute map to return when an element has no attributes.
    //
    const attribute_map_type empty_attr_map_;

    // Return the element entry corresponding to the current depth, if
    // exists, and NULL otherwise.
    //
    const element_entry*
    get_element () const;

    const element_entry*
    get_element_ () const;

    void
    pop_element ();
  };

  LIBSTUDXML_EXPORT std::ostream&
  operator<< (std::ostream&, parser::event_type);
}

#include "parser.ixx"
#include "parser.txx"
//
#include "details/post.hxx"

#endif // LIBSTUDXML_PARSER_HXX
