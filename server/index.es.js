

const h_modulelist=document.getElementById('modulelist');
document.addEventListener("DOMContentLoaded", function(event) {
	var bloborarr=new Uint8Array([tc.listmodules]);
	postreqforarraybuffer(bloborarr,listmodules);
});



function listmodules(arrbuf){
	var off=OFF;
	pred.IS_A_NATURAL_LANG_WORD_SENSE=new Uint8Array(arrbuf,off,8);off+=8;
	pred.VERB_CONTAINS_TEXT=new Uint8Array(arrbuf,off,8);off+=8;
	pred.ANY_USER_DEFINED_VERB=new Uint8Array(arrbuf,off,8);off+=8;
	pred.IS_A_TEXT_NOTE=new Uint8Array(arrbuf,off,8);off+=8;
	pred.IS_A_BOOKMARK=new Uint8Array(arrbuf,off,8);off+=8;
	var strs=decodeutf8strs(new Uint8Array(arrbuf,off));
	assert(!(strs.length%2));
	for(var i=0;i<strs.length;){
		let nm=strs[i];
		++i;
		var desc=strs[i];
		++i;
		h_modulelist.appendChild(createAnchorClelemWithText(desc)).onclick=function() {
			moduleview(nm);
		};
	}
	h_modulelist.appendChild(createAnchorClelemWithText('Switch Text Alignment')).onclick=()=>{//todo save to db? //todo set global rule for input & select?
		switch(document.body.style.textAlign){
		case ''://initial value can be left/right?
		case 'left':
			document.body.style.textAlign='center';
			break;
		case 'center':
			document.body.style.textAlign='right';
			break;
		case 'right':
			document.body.style.textAlign='left';
			break;
		}
	};
	focusonfirst();
}
