#pragma once

namespace CppHttp {
	namespace Net {
		enum class ResponseType {
			// STATUS CODE: 200
			OK = 200,

			// STATUS CODE: 201
			CREATED = 201,

			// STATUS CODE: 400
			BAD_REQUEST = 400,

			// STATUS CODE: 404
			NOT_FOUND = 404,

			// STATUS CODE: 500
			INTERNAL_ERROR = 500,

			// STATUS CODE: 501
			NOT_IMPLEMENTED = 501,

			// STATUS CODE: 401
			NOT_AUTHORIZED = 401,

			// STATUS CODE: 403
			FORBIDDEN = 403,

			// STATUS CODE: 200
			JSON = 1,

			// STATUS CODE: 200
			HTML = 2,

			// STATUS CODE: 200
			TEXT = 0,

			// STATUS CODE: 302
			REDIRECT = 302,

			// STATUS CODE: 409
			ALREADY_EXISTS = 409
		};
	}
}