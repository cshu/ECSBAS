function mkstrforbm(){
	if(document.doctype){
		return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+new XMLSerializer().serializeToString(document.doctype)+document.documentElement.outerHTML;
	}else{
		return document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+document.documentElement.outerHTML;
	}
};
window.open("http://127.0.0.1:28422/cef9fd58ade54ca086ac66669f49dc86",mkstrforbm());
//bookmarklet: javascript:(function(){function%20mkstrforbm(){if(document.doctype){return%20document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+new%20XMLSerializer().serializeToString(document.doctype)+document.documentElement.outerHTML;}else{return%20document.URL.length+'.'+document.URL+document.title.length+'.'+document.title+document.documentElement.outerHTML;}};window.open("http://127.0.0.1:28422/cef9fd58ade54ca086ac66669f49dc86",mkstrforbm());})();
