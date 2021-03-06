#include <iostream>
#include "simdjson.h"

using namespace std;
using namespace simdjson;
using error_code=simdjson::error_code;

void basics_1() {
  const char *filename = "x.txt";

  dom::parser parser;
  dom::element doc = parser.load(filename); // load and parse a file

  cout << doc;
}

void basics_2() {
  dom::parser parser;
  dom::element doc = parser.parse("[1,2,3]"_padded); // parse a string

  cout << doc;
}

void basics_dom_1() {
  auto cars_json = R"( [
    { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
    { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
    { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
  ] )"_padded;
  dom::parser parser;

  // Parse and iterate through each car
  for (dom::object car : parser.parse(cars_json)) {
    // Accessing a field by name
    cout << "Make/Model: " << car["make"] << "/" << car["model"] << endl;

    // Casting a JSON element to an integer
    uint64_t year = car["year"];
    cout << "- This car is " << 2020 - year << "years old." << endl;

    // Iterating through an array of floats
    double total_tire_pressure = 0;
    for (double tire_pressure : car["tire_pressure"]) {
      total_tire_pressure += tire_pressure;
    }
    cout << "- Average tire pressure: " << (total_tire_pressure / 4) << endl;

    // Writing out all the information about the car
    for (auto field : car) {
      cout << "- " << field.key << ": " << field.value << endl;
    }
  }
}



void basics_dom_2() {
  auto cars_json = R"( [
    { "make": "Toyota", "model": "Camry",  "year": 2018, "tire_pressure": [ 40.1, 39.9, 37.7, 40.4 ] },
    { "make": "Kia",    "model": "Soul",   "year": 2012, "tire_pressure": [ 30.1, 31.0, 28.6, 28.7 ] },
    { "make": "Toyota", "model": "Tercel", "year": 1999, "tire_pressure": [ 29.8, 30.0, 30.2, 30.5 ] }
  ] )"_padded;
  dom::parser parser;
  dom::element cars = parser.parse(cars_json);
  cout << cars.at("0/tire_pressure/1") << endl; // Prints 39.9
}

void basics_dom_3() {
  auto abstract_json = R"( [
    {  "12345" : {"a":12.34, "b":56.78, "c": 9998877}   },
    {  "12545" : {"a":11.44, "b":12.78, "c": 11111111}  }
  ] )"_padded;
  dom::parser parser;

  // Parse and iterate through an array of objects
  for (dom::object obj : parser.parse(abstract_json)) {
    for(const auto& key_value : obj) {
      cout << "key: " << key_value.key << " : ";
      dom::object innerobj = key_value.value;
      cout << "a: " << double(innerobj["a"]) << ", ";
      cout << "b: " << double(innerobj["b"]) << ", ";
      cout << "c: " << int64_t(innerobj["c"]) << endl;
    }
  }
}

void basics_dom_4() {
  auto abstract_json = R"(
    {  "str" : { "123" : {"abc" : 3.14 } } } )"_padded;
  dom::parser parser;
  double v = parser.parse(abstract_json)["str"]["123"]["abc"].get<double>();
  cout << "number: " << v << endl;
}


namespace treewalk_1 {
  void print_json(dom::element element) {
    switch (element.type()) {
      case dom::element_type::ARRAY:
        cout << "[";
        for (dom::element child : dom::array(element)) {
          print_json(child);
          cout << ",";
        }
        cout << "]";
        break;
      case dom::element_type::OBJECT:
        cout << "{";
        for (dom::key_value_pair field : dom::object(element)) {
          cout << "\"" << field.key << "\": ";
          print_json(field.value);
        }
        cout << "}";
        break;
      case dom::element_type::INT64:
        cout << int64_t(element) << endl;
        break;
      case dom::element_type::UINT64:
        cout << uint64_t(element) << endl;
        break;
      case dom::element_type::DOUBLE:
        cout << double(element) << endl;
        break;
      case dom::element_type::STRING:
        cout << std::string_view(element) << endl;
        break;
      case dom::element_type::BOOL:
        cout << bool(element) << endl;
        break;
      case dom::element_type::NULL_VALUE:
        cout << "null" << endl;
        break;
    }
  }

  void basics_treewalk_1() {
    dom::parser parser;
    print_json(parser.load("twitter.json"));
  }
}

#ifdef SIMDJSON_CPLUSPLUS17
void basics_cpp17_1() {
  dom::parser parser;
  padded_string json = R"(  { "foo": 1, "bar": 2 }  )"_padded;
  auto [object, error] = parser.parse(json).get<dom::object>();
  if (error) { cerr << error << endl; return; }
  for (auto [key, value] : object) {
    cout << key << " = " << value << endl;
  }
}
#endif

void basics_cpp17_2() {
  // C++ 11 version for comparison
  dom::parser parser;
  padded_string json = R"(  { "foo": 1, "bar": 2 }  )"_padded;
  dom::object object;
  simdjson::error_code error;
  parser.parse(json).get<dom::object>().tie(object, error);
  for (dom::key_value_pair field : object) {
    cout << field.key << " = " << field.value << endl;
  }
}

void basics_ndjson() {
  dom::parser parser;
  for (dom::element doc : parser.load_many("x.txt")) {
    cout << doc["foo"] << endl;
  }
  // Prints 1 2 3
}

void implementation_selection_1() {
  cout << "simdjson v" << STRINGIFY(SIMDJSON_VERSION) << endl;
  cout << "Detected the best implementation for your machine: " << simdjson::active_implementation->name();
  cout << "(" << simdjson::active_implementation->description() << ")" << endl;
}

void implementation_selection_2() {
  for (auto implementation : simdjson::available_implementations) {
    cout << implementation->name() << ": " << implementation->description() << endl;
  }
}

void implementation_selection_3() {
  cout << simdjson::available_implementations["fallback"]->description() << endl;
}

void implementation_selection_4() {
  // Use the fallback implementation, even though my machine is fast enough for anything
  simdjson::active_implementation = simdjson::available_implementations["fallback"];
}

void performance_1() {
  dom::parser parser;

  // This initializes buffers and a document big enough to handle this JSON.
  dom::element doc = parser.parse("[ true, false ]"_padded);
  cout << doc << endl;

  // This reuses the existing buffers, and reuses and *overwrites* the old document
  doc = parser.parse("[1, 2, 3]"_padded);
  cout << doc << endl;

  // This also reuses the existing buffers, and reuses and *overwrites* the old document
  dom::element doc2 = parser.parse("true"_padded);
  // Even if you keep the old reference around, doc and doc2 refer to the same document.
  cout << doc << endl;
  cout << doc2 << endl;
}

#ifdef SIMDJSON_CPLUSPLUS17
SIMDJSON_PUSH_DISABLE_ALL_WARNINGS
// The web_request part of this is aspirational, so we compile as much as we can here
void performance_2() {
  dom::parser parser(1024*1024); // Never grow past documents > 1MB
//   for (web_request request : listen()) {
    auto [doc, error] = parser.parse("1"_padded/*request.body*/);
//     // If the document was above our limit, emit 413 = payload too large
    if (error == CAPACITY) { /* request.respond(413); continue; */ }
//     // ...
//   }
}

// The web_request part of this is aspirational, so we compile as much as we can here
void performance_3() {
  dom::parser parser(0); // This parser will refuse to automatically grow capacity
  simdjson::error_code allocate_error = parser.allocate(1024*1024); // This allocates enough capacity to handle documents <= 1MB
  if (allocate_error) { cerr << allocate_error << endl; exit(1); }

  // for (web_request request : listen()) {
    auto [doc, error] = parser.parse("1"_padded/*request.body*/);
    // If the document was above our limit, emit 413 = payload too large
    if (error == CAPACITY) { /* request.respond(413); continue; */ }
    // ...
  // }
}
SIMDJSON_POP_DISABLE_WARNINGS
#endif

int main() {
  basics_dom_1();
  basics_dom_2();
  basics_dom_3();
  basics_dom_4();
  return 0;
}
