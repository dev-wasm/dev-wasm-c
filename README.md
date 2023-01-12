# Devcontainer WASM-C
Simple devcontainer for C/C++ development

# Usage

## Github Codespaces
Just click the button:

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://github.com/codespaces/new?hide_repo_select=true&ref=main&repo=575629237)

## Visual Studio Code
Note this assumes that you have the VS code support for remote containers and `docker` installed 
on your machine.

```sh
git clone https://github.com/dev-wasm/dev-wasm-c
cd dev-wasm-c
code ./
```

Visual studio should prompt you to see if you want to relaunch the workspace in a container, you do.

# Building and Running

## Basic example
```sh
# compile C
clang -o main.wasm main.c
wasmtime main.wasm

# compile C++
clang++ -o main-cc.wasm main.cc
wasmtime main-cc.wasm
```

## Web serving with WAGI

There is a simple example of web serving via WebAssembly + CGI (WAGI) in
the `webserver` directory. It uses the lighttpd web server and `mod_cgi`.
See the `webserver/lighttpd.conf` file for more details.

```sh
clang -o wagi.wasm webserver/wagi.c
clang++ -o wagi-cc.wasm webserver/wagi.cc -fno-exceptions

lighttpd -D -f webserver/lighttpd.conf
```

Once the server is running, VS Code or Codespaces should prompt you to connect to the open port.

## Http client example
There is a more complicated example in the [`http` directory](./http/) which shows an example 
of making an HTTP client call using the experimental wasi+http support in [`wasmtime-http`](https://github.com/brendandburns/wasmtime).

# Debugging
The easiest way to debug is to just add breakpoints and click on the launch icon, which will launch
the VS Code debugger.

## Command line
If you want to debug in the command line you can do the following:
```sh
lldb wasmtime -- -g main.wasm --dir .
(lldb) target create "wasmtime"
Current executable set to 'wasmtime' (x86_64).
(lldb) settings set -- target.run-args  "-g" "main.wasm" "--dir" "."
(lldb) settings set target.disable-aslr false  # This is needed to debug inside an un-privileged container
(lldb) b main.c:28
Breakpoint 1: no locations (pending).
WARNING:  Unable to resolve breakpoint to any actual locations.
(lldb) run
Process 11490 launched: '/root/.wasmtime/bin/wasmtime' (x86_64)
1 location added to breakpoint 1
Process 11490 stopped
* thread #1, name = 'wasmtime', stop reason = breakpoint 1.1
    frame #0: 0x00007f0440e952c0 JIT(0x5624884a01d0)`main at main.c:28:13
   25  
   26   int main()
   27   {
-> 28       fprintf(stdout, "Hello C World!\n");
   29  
   30       FILE *f = fopen("test.txt", "w+");
   31       if (!f) {
(lldb)
```
