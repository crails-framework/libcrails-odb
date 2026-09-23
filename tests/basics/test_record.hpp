#pragma once
#include <crails/odb/model/base.hpp>
#include <string>

#pragma db object
class TestRecord : public Crails::Odb::ModelBase
{
  odb_instantiable();
public:
  TestRecord() {}
  TestRecord(const std::string& label) : label(label) {}

  const std::string& get_label() const { return label; }
  void set_label(const std::string& value) { label = value; }

private:
  std::string label;
};
