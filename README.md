# Argyle

Argyle is a minimalist webserver that supports just enough functionality to be useful. 
That also gives it the property of being tiny. 

## Build and Run

1. Create a config file at `/etc/argyle.conf`. An example config file is:
   ```
   AccessLogFile /Users/tyler/Sites/argyle/log/access.log
   ErrorLogFile /Users/tyler/Sites/argyle/log/error.log
   BasePath /Users/tyler/Sites/argyle/document_root
   RunAsUserName tyler
   ```
   Argye will look for files in `BasePath`. Do not include a trailing slash.

   If you'd like to change the location of the config file, the path is defined at the top of `main.cpp`.

2. Build with `make`. Run `sudo ./argyle`. Hope it works.

## License

Argyle is released under the [MIT license](https://tldrlegal.com/license/mit-license).
