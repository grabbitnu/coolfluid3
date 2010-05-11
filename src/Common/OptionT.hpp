#ifndef CF_Common_OptionT_hpp
#define CF_Common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>

#include "Common/Option.hpp"
#include "Common/StringOps.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// generic conversion from string to POD
  template < typename TYPE >
  struct ConvertValue
  {
    static TYPE convert ( char * str )
    {
//      CFinfo << "converting string to POD\n" << CFendl;
      std::string ss (str);
      return StringOps::from_str< TYPE >(ss);
    }
  };

  /// specialization to handle vector conversions
  template < typename TYPE >
  struct ConvertValue< std::vector<TYPE> >
  {
    static std::vector<TYPE> convert ( char * str )
    {
//      CFinfo << "converting string to vector\n" << CFendl;

      std::vector < TYPE > vvalues; // start with clean vector

      std::string ss (str);
      boost::tokenizer<> tok (ss);
      for(boost::tokenizer<>::iterator elem = tok.begin(); elem != tok.end(); ++elem)
      {
        vvalues.push_back( StringOps::from_str< TYPE >(*elem) );
      }

      return vvalues; // assign to m_value (replaces old values)
    }
  };

////////////////////////////////////////////////////////////////////////////////

  /// Class defines options to be used in the ConfigObject class
  /// @author Tiago Quintino
  template < typename TYPE >
      class OptionT : public Option  {

  public:

    typedef TYPE value_type;

    OptionT ( const std::string& name, const std::string& desc, value_type def ) :
        Option(name,DEMANGLED_TYPEID(value_type), desc, def)
    {
//      CFinfo
//          << " creating option ["
//          << m_name << "] of type ["
//          << m_type << "] w default ["
////          << StringOps::to_str( def<TYPE>() ) << "] desc ["
//          << m_description << "]\n" << CFendl;
    }

    virtual void change_value ( rapidxml::xml_node<> *node )
    {
      TYPE vt = ConvertValue<TYPE>::convert( node->value() );
      m_value = vt;
    }

  };

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionT_hpp
