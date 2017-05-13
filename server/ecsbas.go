package main

//note windows #cgo LDFLAGS: -L. -llibecsbas

/*
#cgo CXXFLAGS: -x c++ -std=c++14 -pedantic-errors -Wall -Wextra -Wshadow -I/usr/include/SDL2
#cgo LDFLAGS: -lstdc++fs -lSDL2  -L. -lsqlite3
#include <stdlib.h>
#include <libecsbas.h>
*/
import "C"
import(
	"unsafe"
	"os"
	"os/user"
	"path/filepath"
	"io/ioutil"
	"net"
	"net/http"
	"log"
	"sync"
	"bytes"
	"strings"
)

const TCP_PORT_ECSBAS_HTTP=":28422"
const DEFAULT_INTERFACE=""//0.0.0.0
//const DEFAULT_INTERFACE="127.0.0.1"

const TCP_UDP_PORT_ECSBAS_MAIN=28423

func handlerfunc(w http.ResponseWriter, r *http.Request) {
	//sttime:=time.Now()
	//defer func(){log.Println(time.Since(sttime))}()

	//no need to defer recover(). If ServeHTTP panics, the server will recover it and log
	if r.Method=="GET"{
		var filebytes []byte
		var err error
		switch r.URL.Path{
		case "/favicon.ico":
			filebytes,err=ioutil.ReadFile("favicon.ico")
			if err!=nil{
				log.Println(err)
				return
			}
			w.Header().Set("Content-Type", "image/x-icon")
		case "/index.css":
			filebytes,err=ioutil.ReadFile("index.css")
			if err!=nil{
				log.Println(err)
				return
			}
			w.Header().Set("Content-Type", "text/css; charset=UTF-8")
		case "/encoding-indexes.js":
			filebytes,err=ioutil.ReadFile("encoding-indexes.js")
			goto jshe
		case "/encoding.js":
			filebytes,err=ioutil.ReadFile("encoding.js")
			goto jshe
		case "/bundle.js":
			filebytes,err=ioutil.ReadFile("bundle.js")
			goto jshe
		case "/bundle-bm.js":
			filebytes,err=ioutil.ReadFile("bundle-bm.js")
			goto jshe
		case "/cef9fd58ade54ca086ac66669f49dc86":
			filebytes,err=ioutil.ReadFile("bm.htm")
			goto htmhe
		case "/":
			filebytes,err=ioutil.ReadFile("index.htm")
			goto htmhe
		default:
			w.WriteHeader(404)
			return
		}
		goto writefilecont
		htmhe:
		if err!=nil{
			log.Println(err)
			return
		}
		w.Header().Set("Content-Type", "text/html; charset=UTF-8")
		goto writefilecont
		jshe:
		if err!=nil{
			log.Println(err)
			return
		}
		w.Header().Set("Content-Type", "application/javascript; charset=UTF-8")//text/javascript for ie8?
		goto writefilecont
		writefilecont:
		_,err=w.Write(filebytes)
		if err!=nil{
			log.Println(err)
			return
		}
		return
	}

	body, err:=ioutil.ReadAll(r.Body)
	if err!=nil{
		log.Println(err)
		return
	}
	if !bytes.HasPrefix(body,[]byte{0xfd, 0xcf, 0x17, 0xa4, 0xf4, 0xf9, 0x40, 0x28, 0xb7, 0x1b, 0xed, 0x64, 0x1b, 0x2c, 0x3c, 0xb5}) {return}
	blen:=C.longlong(len(body))

	exitl.Lock()
	defer exitl.Unlock()
	if exitf {return;}
	var resultp unsafe.Pointer
	var resultlen C.int
	C.newreq(blen-16,(*C.uchar)(unsafe.Pointer(&body[16])),&resultp,&resultlen)
	if resultlen<0{
		if resultlen==-1{
			//undone db already closed
		}else{
			//undone error
		}
		return
	}
	resultbytes:=C.GoBytes(resultp,resultlen)
	w.Header().Set("Content-Type", "application/octet-stream")
	_, err=w.Write([]byte{0x7d, 0xfe, 0xf3, 0x20, 0x6b, 0x31, 0x42, 0x4f, 0x8b, 0x31, 0x58, 0x1e, 0x22, 0xd1, 0x21, 0x6f})
	if err!=nil{
		log.Println(err)
		return
	}
	_, err=w.Write(resultbytes)
	if err!=nil{
		log.Println(err)
		return
	}
}


//undone export function for c++ to call: downloading from dropbox / google drive to sync db

//remove
//var trch = make(chan []byte)


func reqshutdowndb(){
	exitl.Lock()
	defer exitl.Unlock()
	if exitf {return;}
	C.reqshutdowndb_noexcept()
}


var exitl sync.Mutex
var exitf bool//add this bool bc i'm paranoid, it should be unnecessary
var listener net.Listener
func main() {
	//todo check new release and alert about downlaoding new version?
	log.SetFlags(log.LstdFlags|log.Llongfile)
	e_customizable:=os.Getenv("RESOURCE_STORE_COMM")
	if e_customizable==""{
		usr, err := user.Current()
		if err!=nil{
			log.Println(err)//log.fatal is rarely useful, use panic in most cases
			return//panic(1)
		}
		e_customizable=filepath.Join(usr.HomeDir,"RESOURCE_STORE_COMM")
	}
	cstr := C.CString(e_customizable)
	defer C.free(unsafe.Pointer(cstr))
	if 0!=C.freopen_out_err(cstr){
		return//panic(1)
	}
	///
	logf,err:=os.OpenFile(filepath.Join(e_customizable,"ecsbas","_log","gostandard"),  os.O_RDWR | os.O_CREATE | os.O_APPEND, 0666)//without O_RDWR, log doesn't work on linux?
	if err!=nil{
		log.Println(err);
		return//panic(1)
	}
	defer logf.Close()
	log.SetOutput(logf)
	///
	log.Println(">")
	go func(){
		defer func(){
			if e:=recover(); e !=nil{log.Println(e)}//let's not call reqshutdowndb() here, bc cgo can panic
		}()
		if 1!=C.waituntildbisready(){
			return
		}
		http.HandleFunc("/", handlerfunc)
		var err error
		func(){
			exitl.Lock()
			defer exitl.Unlock()
			var laddr string
			if len(os.Args)<=1{
				laddr=DEFAULT_INTERFACE
			}else{
				laddr=os.Args[1]
				if strings.IndexByte(laddr,':')!=-1{
					listener, err=net.Listen("tcp",laddr)
					return
				}
			}
			listener, err=net.Listen("tcp",laddr+TCP_PORT_ECSBAS_HTTP)
		}();
		if err!=nil{
			//todo if port is used, use another port.
			log.Println(err)
			reqshutdowndb()
			return
		}
		//remove
		//http.ListenAndServe(TCP_PORT_ECSBAS_HTTP, nil)
		log.Println(http.Serve(listener,nil))//undone if listener is just closed, then no need to log?
		//if err=httpServe(listener,nil);err.Error()=="Closed"{}
	}()
	C.showwin()
	exitl.Lock()
	//?not really necessary to close?
	//if listener!=nil{
	//	listener.Close();//undone Go 1.8 will include Server::Shutdown(context.Context) and Server::Close(), these will be better?
	//}
	exitf=true;
}
