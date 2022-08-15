#include "timestamps.hpp"
#include <chrono>

using namespace std;
using namespace Crails;

void Odb::ModelTimestamps::save(odb::database& db)
{
  update_timestamps();
  BaseType::save(db);
}

void Odb::ModelTimestamps::destroy(odb::database& db)
{
  update_timestamps();
  BaseType::destroy(db);
}

void Odb::ModelTimestamps::update_timestamps()
{
  auto now = chrono::system_clock::now();

  updated_at = chrono::system_clock::to_time_t(now);
  if (get_id() == 0)
    created_at = updated_at;
}
