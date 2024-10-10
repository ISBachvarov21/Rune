# Rune
 
## What is Rune?
Rune is a functional C++ web framework, designed for building high-performance, scalable web applications. Rune provides a simple and clean interface while leveraging C++'s speed and the power of functional programming paradigms.
## Features
- Functional Programming: Rune emphasizes a functional approach to building web applications, reducing side effects and promoting cleaner, more modular code.
- Hot Code Reloading: Automatically reload your code during development without restarting the server, allowing for a fast and efficient development workflow.
- Auto Endpoint Linkage: Easily link your routes to controller functions without the need for boilerplate code.
- High Performance: Built on C++ to deliver low-latency and high-throughput performance for web applications.
- Modular and Scalable: Designed to build modular, scalable applications with clear separation of concerns and maintainable code structures.
- Easy configuring: Config for the whole project is managed by [the top level config file](https://github.com/ISBachvarov21/Rune-Template/blob/main/rune.json).

## Getting Started  
### Prerequisites

- C++17 or higher
- CMake 3.8+
- Compiler (e.g., GCC, Clang, or MSVC)

### Installation
<details>
<summary>AUR</summary>

```bash
yay -S rune-cpp-git
```
</details>
<details>
<summary>Build from source</summary>

```bash
git clone https://github.com/ISBachvarov21/Rune
cd Rune
cmake -S . -B build
cmake --build build
cmake --install build
```
</details>

### Initialising a Rune project
```bash
mkdir <new project dir>
cd <new project dir>
rune-cpp --init
```
This will generate a project from [this](https://github.com/ISBachvarov21/Rune-Template) template.

### Running a project with hot reload
Rune acts as its own runtime:
```bash
cd <project dir>
rune-cpp --run
```
After running `rune-cpp --run`, you can head over to `127.0.0.1:8000/` to check if the server is running.  
Default message is "Rune Project".

### Adding endpoints
As specified in the default configuration file (rune.json), the default server location is in `./server/`, and the default location for the endpoints is in `./server/routes/`.  
Endpoints are defined in `.hpp` files. 
They are automatically registered inside `./server/server.cpp`.  
Adding endpoints is done through one of the 4 predefined macros:  
- ROUTE_GET
- ROUTE_POST
- ROUTE_PUT
- ROUTE_DELETE

<br>Example GET endpoint:
```c++
ROUTE_GET("/", root) {
    return { ResponseType::OK, "Hello, World!", {} };
}
```

<br>Instead of "Hello, World!" you can put anything that is stored in `std::string`:
```c++
ROUTE_GET("/", root) {
    std::string response = "This is an example response";
    return { ResponseType::OK, response, {} };
}
```

<br>Macro syntax:
```c++
ROUTE_<ACTION>(<route>, <function suffix>) {
    <function code>
    return {<status>, <response>, <optional response cookies>}; // initialise an std::tuple<CppHttp::Net::ResponseType, std::string, std::optional<std::unordered_map<std::string, std::string>>
}
```

<br>Which expands to:
```c++
HttpResponse <action>_<function suffix>(CppHttp::Net::Request req) {
    <function code>
    return {<status>, <response>, <optional response cookies>};
}
```
<br>And then registered in `server.cpp` as:
```c++
router.AddRoute(<action>, <route>, <action>_<function suffix>)
```

### Accessing the request object
All information about the sent request can be found inside `req.m_info`.  
Example access to request body:
```c++
ROUTE_POST("/user/create", create_user) {
    json body;
    try {
        body = json::parse(req.m_info.body);
    }
    catch (json::parse_error& e) {
        return { ResponseType::BAD_REQUEST, e.what(), {} };
    }
    
    // Store user in database
    
    std::string access_token = ...; // Generate access token for user
    
    json response;
    response["username"] = access_token;
    return { ResponseType::JSON, response.dump(4), {} };
}
```

### Defining path parameters
One can have a dynamic route by surrounding each path parameter with curly braces:
```c++
ROUTE_GET("/user/{user_id}/get", admin_retrieve_user) {
    int user_id = std::stoi(req.m_info.parameters["user_id"]);
    
    // Fetch user from database
    
    json response;
    // Load user fields into response
    
    return { ResponseType::JSON, response.dump(4), {} };
}
```
Each path parameter is stored as a key in `req.m_info.parameters`.

### Editing config
By running `rune-cpp --init`, [a default configuration file](https://github.com/ISBachvarov21/Rune-Template/blob/main/rune.json) is created.

The only field inside that needs to be explained is `endpoint_folder`.  
This field specifies the name for the folder which is being watched for changes to `.hpp` files.  
Inside that folder live all of the header files containing endpoint definitions.  
It lives inside the server folder (location is specified by the `server_location` field in the configuration file).
