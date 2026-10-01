#pragma once
#include <string>
#include <string_view>
#include <type_traits>

namespace Crails
{
  namespace Odb
  {
    namespace TypeTraits
    {
      template<typename T, typename = void>
      struct has_get_db_name : std::false_type {};

      template<typename T>
      struct has_get_db_name<T, std::void_t<decltype(std::declval<const T&>().get_database_name())>>
      {
        using return_type = std::decay_t<decltype(std::declval<const T&>().get_database_name())>;
        static constexpr bool value = std::is_same_v<return_type, std::string> || 
                                      std::is_same_v<return_type, std::string_view>;
      };
    }

    template<typename T>
    constexpr std::string_view get_database_name_for(const T& model)
    {
      if constexpr (TypeTraits::has_get_db_name<T>::value)
        return model.get_database_name();
      else
        return "default";
    }
  }
}
