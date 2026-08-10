#include "nlohmann/json-schema.hpp"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

/*
 * This schema describes a JSON object representing a map from strings to
 * complex numbers, where each complex numbers is either a 2-element tuple-like
 * array of the real and imaginary part or an object with explicitly named
 * "real" and "imag" properties.
 *
 * This complex-number schema is supposed to be strict. Both items or properties
 * are required through the "required" or "minItems" directives and additional
 * items beyond those are denied through "additionalItems": false and
 * "additionalProperties": false.
 */
static auto schema = json::parse(R"JSON({
	"description": "map from from string to complex numbers",
	"additionalProperties": {
		"description": "complex number",
		"type": ["array", "object"],

		"required": ["real", "imag"],
		"additionalProperties": false,
		"properties": {
			"real": {
				"description": "real part",
				"type": "number"
			},
			"imag": {
				"description": "imaginary part",
				"type": "number"
			}
		},

		"minItems": 2,
		"additionalItems": false,
		"items": [
			{
				"description": "real part",
				"type": "number"
			},
			{
				"description": "imaginary part",
				"type": "number"
			}
		]
	}
})JSON");

/*
 * This example is supposed to test the error messages for the unexpected additional
 * item or property in the object.
 *
 * Trivial "additionalItems" or "additionalProperties" have an unambigous meaning of
 * "there are no other properties or items than those which are explicitly named here".
 * 
 * The default error message "instance invalid as per false-schema" doesn't quite
 * reflect this situation as clearly as it could. This will now emit
 * "Found unexpected property/item ... in object/array" instead.
 */
static auto example = json::parse(R"JSON({
	"invalid-tuple": [1, 2, 3],
	"invalid-object": { "real": 1, "imag": 2, "other": 3 }
})JSON");

class error_message_verifier : public nlohmann::json_schema::error_handler {
public:
	void error(const json::json_pointer &, const json &, const std::string &msg) override
	{
		if (not msg.rfind("Found unexpected", 0))
			throw std::runtime_error("Expected an error message that starts with 'Found unexpected ...'");
	}
};

int main(void)
{
	auto validator = json_validator(schema);
	auto verifier = error_message_verifier();
	validator.validate(example, verifier);

	return EXIT_SUCCESS;
}
