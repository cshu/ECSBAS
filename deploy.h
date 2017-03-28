#pragma once

//note you might think single-argument verb is a misnomer (but rather single-parameter), but in predicate logic and language predicate (grammar), the wording "argument" seems more common. Besides, SA verb may be regarded as single-arity/single-ary verb.

#define SENSEID_NOT_REAL 0
#define IS_A_NATURAL_LANG_WORD_SENSE 70001
#define VERB_CONTAINS_TEXT 297314
#define ANY_USER_DEFINED_VERB 297313
#define IS_A_TEXT_NOTE 297312
#define IS_A_BOOKMARK 297311
#define VERB_URL_CONTAINS_TEXT 297310
#define VERB_FILENAME_CONTAINS_TEXT 297309
#define VERB_DESC_CONTAINS_TEXT 297308

#define DESC_IS_A_NATURAL_LANG_WORD_SENSE "Is A Word Group"
#define DESC_VERB_CONTAINS_TEXT "Text"//uses text
#define DESC_ANY_USER_DEFINED_VERB "*"
#define DESC_IS_A_TEXT_NOTE "Is A Text Note"
#define DESC_IS_A_BOOKMARK "Is A Bookmark"
#define DESC_VERB_URL_CONTAINS_TEXT "URL"
#define DESC_VERB_FILENAME_CONTAINS_TEXT "Filename"
#define DESC_VERB_DESC_CONTAINS_TEXT "Desc"
void initecsbasdbobjects(void){
	char *zErrMsg = nullptr;
	int ib;
#define IF_LOG_THROW_0s if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
	begin_tr();
	ib=sqlite3_exec(databaseA, "create table in_entailment(a integer,c integer,primary key(a,c))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table in_mutexgroup(g integer,v integer,primary key(g,v))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table in_singleargv(v integer,d text,primary key(v,d))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table in_singleargp(v integer,s integer,primary key(v,s))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_tncomments(t integer,i integer,primary key(t,i))without rowid", NULL, NULL, &zErrMsg);//the only two-arg verb table currently implemented (sp_essenttext is not counted)? e.g. a note about how to use an english word, or a note about audio, fiction
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_essenttext(g integer,t blob,primary key(g,t))without rowid", NULL, NULL, &zErrMsg);//similar to sp_tncomments, but just text
	IF_LOG_THROW_0s
	//currenly you have is_a verbs like audio, fiction, note, bookmark, but you should have a special table to store anything? not in a defined category? maybe sp_tncomments & sp_essenttext are enough for that?
	ib=sqlite3_exec(databaseA, "create table sp_text_notes(s integer,d text,f text,primary key(s,d,f))without rowid", NULL, NULL, &zErrMsg);//senseid, dir, filename
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_urlbookmar(i integer,u blob,d blob,f blob,primary key(i,u,d,f))without rowid", NULL, NULL, &zErrMsg);//id,url,desc,filename
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_animationp(i integer,prog blob,rank blob,comm blob,primary key(i,prog,rank,comm))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_myanimelis(i integer,mid blob,title blob,english blob,synonyms blob,episodes blob,type blob,primary key(i,mid,title,english,synonyms,episodes,type))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_filestorag(i integer,snm blob,size blob,wtime blob,onm blob,primary key(i,snm,size,wtime,onm))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_setexsense(s integer,m integer,primary key(s,m))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_setcosense(s integer,m integer,primary key(s,m))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_datafsinfo(d integer,i integer,primary key(d,i))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_datastinfo(d integer,i integer,primary key(d,i))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_personifie(i integer,g blob,dborn blob,ddied blob,comm blob,primary key(i,g,dborn,ddied,comm))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_authorinfo(p integer,i integer,primary key(p,i))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_translinfo(p integer,i integer,l integer,primary key(p,i,l))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_mtextficti(i integer,prog blob,rank blob,comm blob,primary key(i,prog,rank,comm))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_audioprodu(i integer,a integer,rank blob,primary key(i,a,rank))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_compileemb(i integer,m integer,d blob,t blob,primary key(i,m,d,t))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table sp_videogames(i integer,a integer,rank blob,primary key(i,a,rank))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table memoizedcfg_t(v text,k text,primary key(v,k))without rowid", NULL, NULL, &zErrMsg);//memoization OR cfg table
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "create table memoizedcfg_i(v integer,k text,primary key(v,k))without rowid", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "insert into memoizedcfg_i values(297315,'AVAIL_SENSE_ID')", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	ib=sqlite3_exec(databaseA, "insert into memoizedcfg_t values('notes','TEXTNOTE_DIR')", NULL, NULL, &zErrMsg);
	IF_LOG_THROW_0s
	end_tr();
#undef IF_LOG_THROW_0s
	//INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargv values(" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ",?)"));)
	//ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("Is-a natural language word sense"), SQLITE_STATIC);
	//if(SQLITE_OK!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//ib=sqlite3_step(sstmt.s.pstmt);
	//if(SQLITE_DONE!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//CATCH_SET_SUE_THROWs(;)
	//INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into memoizedcfg_i values(297315,?)"));)
	//ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("AVAIL_SENSE_ID"), SQLITE_STATIC);
	//if(SQLITE_OK!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//ib=sqlite3_step(sstmt.s.pstmt);
	//if(SQLITE_DONE!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//CATCH_SET_SUE_THROWs(;)
	//INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into memoizedcfg_b values(?,?)"));)
	//ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("notes"), SQLITE_STATIC);
	//if(SQLITE_OK!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, LITERAL_COMMA_LEN("TEXTNOTE_DIR"), SQLITE_STATIC);
	//if(SQLITE_OK!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//ib=sqlite3_step(sstmt.s.pstmt);
	//if(SQLITE_DONE!=ib){dbready=2;LOG_Is(ib) throw 0;}
	//CATCH_SET_SUE_THROWs(;)
}
