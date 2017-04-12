// ==UserScript==
// @name      BMChecker
// @namespace http://cshu.github.io
// @version   1
// @noframes
// @run-at    document-start
// @grant     GM_getValue
// @grant     GM_setValue
// @grant     GM_deleteValue
// @grant     GM_openInTab
// ==/UserScript==


if(window.location.pathname==='/cef9fd58ade54ca086ac66669f49dc86'){
	var ecsbash=GM_getValue("ecsbash")
	if(!ecsbash)return;
	window.name=ecsbash;
	GM_deleteValue('ecsbash');
}else{
	var sc=0x59;//Y
	var ecsbasp='http://127.0.0.1:28422/';
	ecsbasp+='cef9fd58ade54ca086ac66669f49dc86';
	function mkstrforbm(){
		if(document.doctype){
			return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+new XMLSerializer().serializeToString(document.doctype)+document.documentElement.outerHTML;
		}else{
			return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+document.documentElement.outerHTML;
		}
	}
	window.addEventListener('keydown',function(ke){
		if(ke.ctrlKey&&ke.keyCode===sc){
			GM_setValue('ecsbash',mkstrforbm());
			GM_openInTab(ecsbasp,false);
		}
	},true);
}
