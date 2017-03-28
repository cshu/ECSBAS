# ECSBAS
Local database of bookmarks, HTML archives, and note taking. (Tagged with verbs.)

Usage: http://127.0.0.1:28422 is the default address to access from web browser. RESOURCE_STORE_COMM environment variable is used as the path to store all text notes, HTML archives, database, and logs. If RESOURCE_STORE_COMM is not set, home directory is used instead. Command line argument may specify an interface (with optional port) to listen on. E.g. 127.0.0.1 for local access, and 0.0.0.0 for public access. (Always listen on local interface. Don't use other interfaces unless you are sure it's secure!)

How to build:
Remove the debugging printing for errors in JavaScript
Note the flags for writing GUI binary instead of console binary. `go install -ldflags -H=windowsgui github.com/cshu/ecsbas`
(change `DEFAULT_INTERFACE` constant to build alternative versions of executable)

Acknowledgements:
This work uses SQLite (http://sqlite.org/).
This work uses text-encoding (https://github.com/inexorabletash/text-encoding).
This work uses Reset CSS (http://meyerweb.com/eric/tools/css/reset/).
This work uses Simple DirectMedia Layer (http://libsdl.org/) which is under zlib license.
This work uses Cousine (https://fonts.google.com/specimen/Cousine/) which is under Apache License, Version 2.0.

