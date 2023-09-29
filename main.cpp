#include <main.h>
const void *res;
int reslen;//0 no result, -1 db is already shut down, if<=-2 error
static void errorLogCallback(void *pArg, int iErrCode, const char *zMsg)noexcept{
	try{
		lock_guard<mutex> lgcs{logm};
		STD_CLOG_TIME_FILE_FUNC_LINE<<"SQLite: "<<iErrCode<<' '<<zMsg<<'\n'<<std::flush;
	}catch(...){}
}
#include <sqlite3minwrapper.h>
#include <sqlitewc.h>
#include <deploy.h>

#define THROW_IF_INVALID_REQUESTs ASSERT_THROWs//checking for invalid request, can be turned off at compile-time



extern "C" CPPRS_LINK_WITH_C void showwin(void);
extern "C" CPPRS_LINK_WITH_C void newreq(long long,unsigned char *,void **,int *);
extern "C" CPPRS_LINK_WITH_C unsigned char waituntildbisready(void);
extern "C" CPPRS_LINK_WITH_C unsigned char freopen_out_err(char *);
extern "C" CPPRS_LINK_WITH_C void reqshutdowndb_noexcept(void);



#define MNUMOFRECORDS 1024//4096//256 //todo this really should be a smaller number like 256, but there is no pagination for now, so just use a big number
static_assert(INT_MAX>1000000000,"");//do not support some architectures for the convenience of sqlite3_column_bytes

#define CHBUFSI 0x100//debug change to 0x1024
#define SYNOPSISLEN 0xff
#include <search.h>

unsigned char waituntildbisready(void){
	try{
		unique_lock<mutex> ulp{fem};
		while(!dbready)
			fecv.wait(ulp);
		return dbready;
	}catch(const std::exception &e){
		LOG_E_NOEXCEPTs(e)
	}catch(...){
		LOG_ERR_NOEXCEPTs
	}
	return 2;
}

void newreq(long long blen,unsigned char *body,void ** resultp,int *resultlen){
	static mutex newrm;//useful only bc newreq can be called by sdl thread?
	try{
		lock_guard<mutex> lgcs{newrm};
		{
			lock_guard<mutex> lg{dbm};
			if(reslen<0){//this is necessary bc after the db is closed and before the http server is closed, a goroutine could reach here
				*resultlen=reslen;
				return;
			}
			rb=body;
			rl=blen;
		}
		dbcv.notify_one();

		unique_lock<mutex> ulp(fem);
		while(rl)
			fecv.wait(ulp);
		*resultp=(void *)res;
		*resultlen=reslen;

		//
		return;
	}catch(const std::exception &e){
		LOG_E_NOEXCEPTs(e)
	}catch(...){
		LOG_ERR_NOEXCEPTs
	}
	*resultlen=-2;
}

//template<class T1,class T2>
void setresnotifyfe(const void *lres,int lreslen){
	{
		lock_guard<mutex> lg{fem};
		rl=0;
		res=lres;
		reslen=lreslen;
	}
	fecv.notify_one();
}

void sdl_pushevent_userevent(void){
	SDL_Event event;
	event.type = SDL_USEREVENT;//fixme SDL_RegisterEvents should be used to avoid conflict when multiple modules depend on SDL. (if you have only one module using SDL then i guess it doesn't bite you.)
	//event.user.code = 0;//? not necessary to set any other member?
	if(!SDL_PushEvent(&event)){
		LOG_ERRs//cannot call SDL_GetError bc it's not SDL thread?
		//todo in this case, call go function to close listener?
	}
}

void db(void)noexcept{
	try{
		//
		unique_lock<mutex> ulp(dbm);
		INIT_TRYs(dtor_sqlite_close dtor_sqlite_close_;)
		//INIT_TRYs(dtor_sqlite_commit dtor_sqlite_commit_;)
		//INIT_TRYs(dtor_update_avail_sense_id dtor_update_avail_sense_id_;)
		{
			lock_guard<mutex> lgp{fem};

			auto dbp=e_p;
			dbp/="_db";
			dbp/="main";
			char *zErrMsg = nullptr;
			auto ib=sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
			if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
			ib=sqlite3_config(SQLITE_CONFIG_LOG, errorLogCallback, NULL);
			if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
			auto filefoundbeforeopen = is_regular_file(symlink_status(dbp));
			ib=sqlite3_open(dbp.u8string().c_str(), &databaseA);//note "A database connection handle is usually returned in *ppDb, even if an error occurs."
			if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA locking_mode = EXCLUSIVE", NULL, NULL, &zErrMsg);
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);//when it's OFF, rollback cannot be used.
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA cache_size = 90000000", NULL, NULL, &zErrMsg);//90GB in-memory (with default page_size)
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA temp_store = MEMORY", NULL, NULL, &zErrMsg);
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			ib=sqlite3_exec(databaseA, "PRAGMA automatic_index = 0", NULL, NULL, &zErrMsg);//main reason to turn this off is that my WITH clause causes (284) automatic index warning!
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			//ib=sqlite3_exec(databaseA, "BEGIN EXCLUSIVE", NULL, NULL, &zErrMsg);//note using a single transaction is probably faster but not safe?
			//if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			//dtor_sqlite_commit_.begun=true;
			if(!filefoundbeforeopen) initecsbasdbobjects();
			INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("SELECT v FROM memoizedcfg_i WHERE k='AVAIL_SENSE_ID'"));)
			ib=sqlite3_step(sstmt.s.pstmt);
			if(SQLITE_ROW!=ib){LOG_Is(ib) throw 0;}
			availid=sqlite3_column_int64(sstmt.s.pstmt, 0);
			CATCH_SET_SUE_THROWs(;)
			ib=sqlite3_exec(databaseA, "create temp table tmptab_v(v integer,d text,primary key(v,d))without rowid", NULL, NULL, &zErrMsg);
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			#define SQL_ID_N_TEXT(i) XSTR(i) ",'" DESC_##i "'"
			ib=sqlite3_exec(databaseA, "insert into tmptab_v values(" SQL_ID_N_TEXT(IS_A_NATURAL_LANG_WORD_SENSE) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(VERB_CONTAINS_TEXT) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(IS_A_TEXT_NOTE) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(IS_A_BOOKMARK) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(VERB_URL_CONTAINS_TEXT) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(VERB_FILENAME_CONTAINS_TEXT) ")"
			";insert into tmptab_v values(" SQL_ID_N_TEXT(VERB_DESC_CONTAINS_TEXT) ")"
			";create temp table tmptab_i0(z integer)"
			";create temp table tmptab_ii0(z integer,y integer)"
			";create temp table tmptab_b0(z blob)", NULL, NULL, &zErrMsg);
			if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
			
			
			dbready=DB_INIT_READY;
		}
		fecv.notify_all();//notify both server goroutine and sdl thread.
		//void *resfs;//free store
		//int resfslen;
		vector<unsigned char> vb;
		begin_db_l:
		for(;;){//loop to receive requests
			while(!rl)
				dbcv.wait(ulp);
			//todo not safe, malicious request can cause UB when you read the buffer without bounds checking
			switch(rb[0]){
			case shutdown:{
					//may be a command from web app, or a goroutine error
					goto after_db_close;
				}
				break;
			case listmodules:{
					vb.clear();
					sqlite3_int64 predefined[]={IS_A_NATURAL_LANG_WORD_SENSE,VERB_CONTAINS_TEXT,ANY_USER_DEFINED_VERB,IS_A_TEXT_NOTE,IS_A_BOOKMARK};
					vb.insert(vb.end(),(unsigned char *)predefined,(unsigned char *)(&predefined+1));
					insert_end_literal(vb,"bookmark\0Bookmarks\0text_note\0Text Notes\0search\0Search\0verbmgmt\0Verb Management\0wordsense\0Word Sense");
					//insert_end_literal(vb,"bookmark\0Bookmarks\0text_note\0Text Notes\0search\0Search\0temlist\0Temporary Lists\0addremoveorreordermodule\0Add, Remove, or Reorder Module\0verbmgmt\0Verb Management\0filesystem\0Filesystem\0wordsense\0Word Sense\0people\0People\0anime\0Anime\0audio\0Audio\0fiction\0Fiction\0software\0Software\0filetransfer\0File Transfer\0messenger\0LAN messenger");
				}
				break;
#					define OFF 2
			case unusualcommands:{
					switch(rb[1]){
						//case 0:{//arbitrary sql//? is it better to respond with failure message instead of throw exception?
						//	auto corb=rb+OFF;
						//	auto sqlsi=strlen((char *)corb)+1;
						//	INIT_TRYs(slw_prestmt sstmt(databaseA, corb, sqlsi);)
						//	corb+=sqlsi;
						//	for(int i=1;;++i){//undone
						//		switch(*corb){
						//			case '\0':
						//				goto end_of_arbitrary_sql_binding;
						//			case 'b':{//blob
						//				auto blen=u32frombytesle(corb);
						//				corb+=sizeof blen;
						//				auto ib=sqlite3_bind_blob(sstmt.s.pstmt, i, corb,blen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
						//				if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						//				corb+=blen;
						//			}break;
						//			//case 'd'://double
						//			case 'I':{//int64
						//				sqlite3_int64 id;
						//				memcpy(&id,corb,sizeof id);
						//				auto ib=sqlite3_bind_int64(sstmt.s.pstmt, i, id);
						//				if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						//				corb+=sizeof id;
						//			}break;
						//			case 't':{
						//				auto slen=strlen((char *)corb);
						//			}break;
						//			//case 'z'://zeroblob
						//		}
						//	}
						//	end_of_arbitrary_sql_binding:;
						//	CATCH_SET_SUE_THROWs(;)
						//}break;
						case 1:{
						}break;
					}
				}
				break;
			case bookmark:{
					switch(rb[1]){
					case (unsigned char)'U':{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							auto urllen=strlen((char *)rb+OFF+sizeof id);
							auto desclen=strlen((char *)rb+OFF+sizeof id+urllen+1);
							auto htmllen=u32frombytesle(rb+OFF+sizeof id+urllen+1+desclen+1);
							size_t off=OFF+sizeof id+urllen+1+desclen+1+sizeof(uint32_t)+htmllen;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from(select * from tmptab_v union all select * from in_singleargv)where d=?"));)
							for(;;){
								auto newdesclen=strlen((char *)rb+off);
								if(!newdesclen)break;
								auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+off,newdesclen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								off+=newdesclen+1;
								ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:
									setresnotifyfe(LITERAL_COMMA_LEN("\1Duplicate description found in db."));
									goto begin_db_l;
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								sqlite3_reset(sstmt.s.pstmt);
							}++off;
							CATCH_SET_SUE_THROWs(;)
							auto bmp=e_p;
							bmp/="_bookmark";
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select(select f from sp_urlbookmar where i=?1),(select 0 from sp_urlbookmar where u=?2 and i<>?1)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, rb+OFF+sizeof id,urllen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_ROW!=ib){LOG_Is(ib) throw 0;}
							if(SQLITE_NULL!=sqlite3_column_type(sstmt.s.pstmt,1)){
								setresnotifyfe(LITERAL_COMMA_LEN("\1Duplicate URL found in DB."));
								goto begin_db_l;
							}
							switch(sqlite3_column_type(sstmt.s.pstmt,0)){
							case SQLITE_BLOB:{
									if(off==rl){
										begin_tr();DELETEALLSINGLEARGPROPSOFSENSs(id)
									}else{
										RECEIVESINGLEARGPROPSCHKANDINSERTs(off,id,{begin_tr();})
									}
									if(htmllen){
										auto newfilenm=makefilenmfromurl((char *)rb+OFF+sizeof id);
										auto filenmlen=sqlite3_column_bytes(sstmt.s.pstmt, 0);
										if(filenmlen){
											auto filenm=(const char *)sqlite3_column_blob(sstmt.s.pstmt, 0);
											bmp.append(filenm,filenm+filenmlen);bmp+=".htm";
											if(!remove(bmp)){LOG_ERRs throw 0;}//even if memicmp returns false, you don't know if the new and old filenms are equivalent on the os
											if(filenmlen==newfilenm.size()){
												UNSAFE_MEMICMPu(cmpbmfilenm,filenm,newfilenm.c_str(),filenmlen,{
													ensure_bm_filenm_is_unique(newfilenm);
												},{})
											}else ensure_bm_filenm_is_unique(newfilenm);
											bmp.replace_filename(newfilenm);
										}else{ensure_bm_filenm_is_unique(newfilenm);bmp/=newfilenm;}
										bmp+=".htm";
										INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update sp_urlbookmar set u=?,d=?,f=? where i=?"));)
										ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF+sizeof id,urllen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, rb+OFF+sizeof id+urllen+1,desclen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_bind_blob(sstmt.s.pstmt, 3, newfilenm.c_str(),newfilenm.size(), SQLITE_TRANSIENT);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_bind_int64(sstmt.s.pstmt, 4, id);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_step(sstmt.s.pstmt);
										if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
										CATCH_SET_SUE_THROWs(;)
									}else{
										INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update sp_urlbookmar set u=?,d=? where i=?"));)
										ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF+sizeof id,urllen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, rb+OFF+sizeof id+urllen+1,desclen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_bind_int64(sstmt.s.pstmt, 3, id);
										if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
										ib=sqlite3_step(sstmt.s.pstmt);
										if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
										CATCH_SET_SUE_THROWs(;)
									}
								}break;
							case SQLITE_NULL:{
									if(off==rl){
										begin_tr();id=ret_avail_globalid();
									}else{//optimize deletion is noop
										RECEIVESINGLEARGPROPSCHKANDINSERTs(off,id,{begin_tr();id=ret_avail_globalid();})
									}
									string newfilenm;
									if(htmllen){
										newfilenm=makeuniquefilenmfromurl((char *)rb+OFF+sizeof id);
										bmp/=newfilenm+".htm";
									}
									INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into sp_urlbookmar values(?,?,?,?)"));)
									ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, rb+OFF+sizeof id,urllen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									ib=sqlite3_bind_blob(sstmt.s.pstmt, 3, rb+OFF+sizeof id+urllen+1,desclen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									ib=sqlite3_bind_blob(sstmt.s.pstmt, 4, newfilenm.c_str(),newfilenm.size(), SQLITE_TRANSIENT);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									ib=sqlite3_step(sstmt.s.pstmt);
									if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
									CATCH_SET_SUE_THROWs(;)
								}break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							off=OFF+sizeof id+urllen+1+desclen+1+sizeof(uint32_t)+htmllen;
							INIT_TRYs(slw_prestmt vstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargv values(?,?)"));)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargp values(?,?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 2, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							for(;;){
								auto newdesclen=strlen((char *)rb+off);
								if(!newdesclen)break;
								auto newid=ret_avail_globalid();
								ib=sqlite3_bind_int64(vstmt.s.pstmt, 1, newid);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_bind_text(vstmt.s.pstmt, 2, (char *)rb+off,newdesclen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								off+=newdesclen+1;
								ib=sqlite3_step(vstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								sqlite3_reset(vstmt.s.pstmt);
								ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, newid);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								sqlite3_reset(sstmt.s.pstmt);
							}
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
							update_avail_sense_id();//for both possible new Boommark and new Verbs
							if(htmllen){
								ofstream ofstre_(bmp,ios_base::binary);
								if(!ofstre_){LOG_ERRs throw 0;}//? should not exit app bc read file can fail for various reasons?
								ofstre_.write((char *)rb+OFF+sizeof id+urllen+1+desclen+1+sizeof(uint32_t),htmllen);
								if(!ofstre_){LOG_ERRs throw 0;}
								ofstre_.close();//dtor doesn't throw even if failbit is set
								if(!ofstre_){LOG_ERRs throw 0;}
							}
							end_tr();
							vb.clear();
						}
						break;
					case (unsigned char)'c':
						vb.clear();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select d,f,i from sp_urlbookmar where u=?"));)
						auto urllen=u32frombytesle(rb+OFF);
						auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF+sizeof(uint32_t),urllen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						switch(ib){
						case SQLITE_ROW:{
								auto id=sqlite3_column_int64(sstmt.s.pstmt, 2);
								vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
								ib=sqlite3_column_bytes(sstmt.s.pstmt, 0);
								resize_write_u32le(vb,ib);
								auto pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,0);
								vb.insert(vb.end(),pcc,pcc+ib);
								selectprops(vb,id);
								vb.resize(vb.size()+sizeof(sqlite3_int64));
								auto filenmlen=sqlite3_column_bytes(sstmt.s.pstmt, 1);
								if(filenmlen){
									auto filenm=(const char *)sqlite3_column_blob(sstmt.s.pstmt, 1);
									auto bmp=e_p;
									bmp/="_bookmark";
									bmp.append(filenm,filenm+filenmlen);
									//auto u8str=bmp.u8string();
									//vb.insert(vb.end(),(unsigned char *)u8str.c_str(),(unsigned char *)u8str.c_str()+u8str.size());
									vb.insert(vb.end(), reinterpret_cast<const unsigned char*>(filenm), reinterpret_cast<const unsigned char*>(filenm) + filenmlen);
									bmp+=".htm";
									auto fsize=file_size(bmp);
									if(fsize==rl-OFF-sizeof(uint32_t)-urllen){
										if(!fsize){vb.push_back(0);break;}
										unique_ptr<char[]> fcbuf(new char[fsize]);
										ifstream ifstre_(bmp,ios_base::binary);//?call before .rdbuf()->pubsetbuf(_buffer, BufferSize); before opening file for better performance?
										if(!ifstre_){LOG_ERRs throw 0;}//? should not exit app bc read file can fail for various reasons?
										ifstre_.read(fcbuf.get(),fsize);
										if(!ifstre_){
											if(!ifstre_.eof()){LOG_ERRs throw 0;}
										}else{
											vb.push_back(memcmp(rb+rl-fsize,fcbuf.get(),fsize));
											break;
										}
									}
								}
								vb.push_back(1);
							}break;
						case SQLITE_DONE:
							break;
						default:{LOG_Is(ib) throw 0;}
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					case (unsigned char)'S':
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select u,d,f from sp_urlbookmar where i=?"));)
						sqlite3_int64 id;
						memcpy(&id,rb+OFF,sizeof id);
						auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						switch(ib){
						case SQLITE_ROW:{
								vb.clear();
								auto ib=sqlite3_column_bytes(sstmt.s.pstmt, 0);
								resize_write_u32le(vb,ib);
								auto pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,0);
								vb.insert(vb.end(),pcc,pcc+ib);
								ib=sqlite3_column_bytes(sstmt.s.pstmt, 1);
								resize_write_u32le(vb,ib);
								pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,1);
								vb.insert(vb.end(),pcc,pcc+ib);
								ib=sqlite3_column_bytes(sstmt.s.pstmt, 2);
								resize_write_u32le(vb,ib);
								pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,2);
								vb.insert(vb.end(),pcc,pcc+ib);
								selectprops(vb,id);
							}break;
						case SQLITE_DONE:
							setresnotifyfe(LITERAL_COMMA_LEN("\0\0\0\0Record missing. Might be already deleted."));
							goto begin_db_l;
						default:{LOG_Is(ib) throw 0;}
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					case (unsigned char)'s':
						vb.clear();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from sp_urlbookmar where instr(lower(u),lower(?1)) or instr(lower(d),lower(?1))"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						for(;;){
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
									vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
									ib=sqlite3_column_bytes(sstmt.s.pstmt, 1);
									resize_write_u32le(vb,ib);
									auto pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,1);
									vb.insert(vb.end(),pcc,pcc+ib);
									ib=sqlite3_column_bytes(sstmt.s.pstmt, 2);
									resize_write_u32le(vb,ib);
									pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,2);
									vb.insert(vb.end(),pcc,pcc+ib);
									ib=sqlite3_column_bytes(sstmt.s.pstmt, 3);
									resize_write_u32le(vb,ib);
									pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,3);
									vb.insert(vb.end(),pcc,pcc+ib);
								}continue;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					case (unsigned char)'d':{
							auto bmp=e_p;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select f from sp_urlbookmar where u=?"));)
							auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									auto filenmlen=sqlite3_column_bytes(sstmt.s.pstmt, 0);
									if(filenmlen){
										auto filenm=(const char *)sqlite3_column_blob(sstmt.s.pstmt, 0);
										bmp/="_bookmark";
										bmp.append(filenm,filenm+filenmlen);
										bmp+=".htm";
									}
								}break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Record missing. Might be already deleted."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from sp_urlbookmar where u=?"));)
							auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							if(bmp!=e_p)
								if(!remove(bmp)){LOG_ERRs throw 0;}
							end_tr();
							vb.clear();
						}break;
					case (unsigned char)'D':{
							auto bmp=e_p;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select f from sp_urlbookmar where u=?"));)
							auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									auto filenmlen=sqlite3_column_bytes(sstmt.s.pstmt, 0);
									if(!filenmlen){
										setresnotifyfe(VALID_PH_PTR,0);
										goto begin_db_l;
									}
									auto filenm=(const char *)sqlite3_column_blob(sstmt.s.pstmt, 0);
									bmp/="_bookmark";
									bmp.append(filenm,filenm+filenmlen);
									bmp+=".htm";
								}break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Record missing. Might be already deleted."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update sp_urlbookmar set f=x'' where u=?"));)
							auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							if(!remove(bmp)){LOG_ERRs throw 0;}
							end_tr();
							vb.clear();
						}break;
					case (unsigned char)'u':
						//update sp_urlbookmar set v=? where k=?
						break;
					}
				}
				break;
			case text_note:{
					//Alternative implementation: default directory of text files is $ECSBAS_DATA_DIR/ecsbas/notes, and user can add any number of additional folder names to memoizedcfg_t or config file for choosing in GUI. (store in db table for the convenience of uploading/sync. store in config file for local machine preference.)
					switch(rb[1]){
					case (unsigned char)'f':
						vb.clear();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a as(select v from memoizedcfg_t where k='TEXTNOTE_DIR')select v from(select v,0 b from a union all select distinct d,1 from sp_text_notes where d not in a)order by b limit " XSTR(MNUMOFRECORDS)));)
						for(;;){
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,0);
									vb.insert(vb.end(),pcc,pcc+sqlite3_column_bytes(sstmt.s.pstmt,0)+1);
								} continue;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						vb.resize(vb.size()-1);
						break;
					case (unsigned char)'p':
						vb.resize(4);
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a as(select v from memoizedcfg_t where k='TEXTNOTE_DIR')select v from(select v,0 b from a union all select distinct d,1 from sp_text_notes where d not in a)order by b limit " XSTR(MNUMOFRECORDS)));)
						for(uint32_t u=0;;++u){
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									ib=sqlite3_column_bytes(sstmt.s.pstmt,0);
									resize_write_u32le(vb,ib);
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,0);
									vb.insert(vb.end(),pcc,pcc+ib);
								}
								continue;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							write_u32le(vb.data(),u);
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k in('TEXTNOTE_MAIN_EDITOR','TEXTNOTE_EDITOR')order by k desc limit " XSTR(MNUMOFRECORDS)));)
						//auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("TEXTNOTE_MAIN_EDITOR"), SQLITE_STATIC);
						//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						//ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, LITERAL_COMMA_LEN("TEXTNOTE_EDITOR"), SQLITE_STATIC);
						//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						for(;;){
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									ib=sqlite3_column_bytes(sstmt.s.pstmt,0);
									resize_write_u32le(vb,ib);
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,0);
									vb.insert(vb.end(),pcc,pcc+ib);
								}
								continue;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					case (unsigned char)'i':{
#ifdef _WIN32
#define RUNEXTERNALCMD "run-external-cmd "
#else
#define RUNEXTERNALCMD "./run-external-cmd "
#endif
							string systemstr(RUNEXTERNALCMD);//todo call go function directly, bc unlike shared library, executable may not be found even if it's under the same directory as this program. Or alternatively, use filesystem::system_complete(argv[0]) to find directory of program file?
							auto notep=e_p;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k='TEXTNOTE_DIR' and not exists(select * from sp_text_notes where v=d and f collate nocase =?)"));)
							
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									notep/=(char *)sqlite3_column_text(sstmt.s.pstmt,0);
									break;
								}
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Filename is already used."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR'"));)
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Main editor not found."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							systemstr+=(const char *)sqlite3_column_text(sstmt.s.pstmt,0);
							systemstr+=' ';
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							auto newnoteid=ret_avail_globalid();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into sp_text_notes values(?,(select v from memoizedcfg_t where k='TEXTNOTE_DIR'),?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, newnoteid);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							update_avail_sense_id(); end_tr();
							notep.append((char *)rb+OFF,(char *)rb+rl);
							{
								fstream fstre_(notep.c_str(),ios_base::app|ios_base::binary);//msvc non-std ctor like _wfopen
								if(!fstre_){LOG_ERRs throw 0;}
								fstre_.close();//dtor doesn't throw even if failbit is set
								if(!fstre_){LOG_ERRs throw 0;}
							}
							system((systemstr+notep.u8string()).c_str());//?path might contain non-ascii, is it okay?
							//todo if ret of system is not 0, alert?
							vb.assign(1,0); vb.insert(vb.end(),(unsigned char *)&newnoteid,(unsigned char *)(&newnoteid+1));
							break;
						}
					case (unsigned char)'I':{
							string systemstr(RUNEXTERNALCMD);
							auto notep=e_p;
							auto dnmlen=strlen((char *)rb+OFF);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from sp_text_notes where d=? and f collate nocase =?"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,dnmlen, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF+dnmlen+1,rl-OFF-dnmlen-1, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Filename is already used."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR'"));)
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Main editor not found."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							systemstr+=(const char *)sqlite3_column_text(sstmt.s.pstmt,0);
							systemstr+=' ';
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							auto newnoteid=ret_avail_globalid();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into sp_text_notes values(?,?,?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, newnoteid);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF,dnmlen, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 3, (char *)rb+OFF+dnmlen+1,rl-OFF-dnmlen-1, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							update_avail_sense_id(); end_tr();
							notep.append((char *)rb+OFF,(char *)rb+OFF+dnmlen);
							if(!exists(notep)){
								if(!create_directory(notep)){LOG_ERRs throw 0;}
							}
							notep.append((char *)rb+OFF+dnmlen+1,(char *)rb+rl);
							{
								fstream fstre_(notep.c_str(),ios_base::app|ios_base::binary);//msvc non-std ctor like _wfopen
								if(!fstre_){LOG_ERRs throw 0;}
								fstre_.close();//dtor doesn't throw even if failbit is set
								if(!fstre_){LOG_ERRs throw 0;}
							}
							system((systemstr+notep.u8string()).c_str());//?path might contain non-ascii, is it okay?
							//todo if ret of system is not 0, alert?
							vb.assign(1,0); vb.insert(vb.end(),(unsigned char *)&newnoteid,(unsigned char *)(&newnoteid+1));
							break;
						}
					case (unsigned char)'s':{
							THROW_IF_INVALID_REQUESTs(rl-OFF>1)
							THROW_IF_INVALID_REQUESTs(CHBUFSI>=2*(rl-OFF-1))
							vb.clear();
							unique_ptr<char[]> chbuf(new char[CHBUFSI]);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from sp_text_notes"));)
							for(auto numofsleft=MNUMOFRECORDS;;){
								auto ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
										auto notep=e_p;
										auto dirnm=sqlite3_column_text(sstmt.s.pstmt,1);
										notep/=(char *)dirnm;
										//recover bad state, dir should already exist
										if(!exists(notep)){
											if(!create_directory(notep)){LOG_ERRs throw 0;}
										}
										//
										auto filenm=sqlite3_column_text(sstmt.s.pstmt,2);
										notep/=(char *)filenm;
										//recover bad state, file doesn't exist
										if(!exists(notep)){
											fstream fstre_(notep.c_str(),ios_base::app|ios_base::binary);
											if(!fstre_){LOG_ERRs throw 0;}
											fstre_.close();
											if(!fstre_){LOG_ERRs throw 0;}
										}
										//
										ifstream ifstre_(notep.c_str(),ios_base::binary);//?call before .rdbuf()->pubsetbuf(_buffer, BufferSize); before opening file for better performance?
										if(!ifstre_){LOG_ERRs throw 0;}//? should not exit app bc read file can fail for various reasons?
										vb.resize(vb.size()+SYNOPSISLEN);
										ifstre_.read((char *)vb.data()+vb.size()-SYNOPSISLEN,SYNOPSISLEN);
										if(!ifstre_ && !ifstre_.eof()){LOG_ERRs throw 0;}
										ib=sqlite3_column_bytes(sstmt.s.pstmt,2);
										if(ib>=rl-OFF){
											UNSAFE_MEMIMEMu(setextnotefn,filenm,ib,rb+OFF,rl-OFF,{},{goto write_note_info;})
										}
										if(ifstre_.gcount()<rl-OFF)goto revert_synopsis;
										UNSAFE_MEMIMEMu(setextnotesyno,vb.data()+vb.size()-SYNOPSISLEN,ifstre_.gcount(),rb+OFF,rl-OFF,{},{goto write_note_info;})
										if(ifstre_.eof())goto revert_synopsis;
										memcpy(chbuf.get(),vb.data()+vb.size()-(rl-OFF-1),rl-OFF-1);
										for(;;){
											if(!ifstre_.read(chbuf.get()+rl-OFF-1,CHBUFSI-(rl-OFF-1))){
												if(!ifstre_.eof()){LOG_ERRs throw 0;}
												if(!ifstre_.gcount())goto revert_synopsis;//if(rl-OFF-1+ifstre_.gcount()<rl-OFF)
											}
											UNSAFE_MEMIMEMu(setextnote,chbuf.get(),rl-OFF-1+ifstre_.gcount(),rb+OFF,rl-OFF,{},{goto write_note_info;})
											if(ifstre_.eof())goto revert_synopsis;
											memcpy(chbuf.get(),chbuf.get()+CHBUFSI-(rl-OFF-1),rl-OFF-1);
										}
										write_note_info:
										ifstre_.clear(); ifstre_.close();
										if(!ifstre_){LOG_ERRs throw 0;}
										{
											auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
											vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
										}
										vb.insert(vb.end(),dirnm,dirnm+sqlite3_column_bytes(sstmt.s.pstmt,1));
										vb.push_back('/');
										vb.insert(vb.end(),filenm,filenm+sqlite3_column_bytes(sstmt.s.pstmt,2)+1);
										if(!numofsleft)break;
										--numofsleft;
										continue;
										revert_synopsis:
										ifstre_.clear(); ifstre_.close();
										if(!ifstre_){LOG_ERRs throw 0;}
										{vb.resize(vb.size()-SYNOPSISLEN);continue;}
									}
								case SQLITE_DONE:break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					case (unsigned char)'e':{
							string systemstr(RUNEXTERNALCMD);
							auto notep=e_p;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR'"));)
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Main editor not found."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							systemstr+=(const char *)sqlite3_column_text(sstmt.s.pstmt,0);
							systemstr+=' ';
							CATCH_SET_SUE_THROWs(;)
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select d,f from sp_text_notes where s=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("File not found."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							notep/=(const char *)sqlite3_column_text(sstmt.s.pstmt,0);
							notep/=(const char *)sqlite3_column_text(sstmt.s.pstmt,1);
							CATCH_SET_SUE_THROWs(;)
							system((systemstr+notep.u8string()).c_str());//?path might contain non-ascii, is it okay?
							//todo if ret of system is not 0, alert?
							vb.clear();
						}
						break;
					case (unsigned char)'d':{
							auto oldp=e_p;
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select d,f from sp_text_notes where s=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Record missing. Might be already deleted."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							oldp/=(char *)sqlite3_column_text(sstmt.s.pstmt,0);
							oldp/=(char *)sqlite3_column_text(sstmt.s.pstmt,1);
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_singleargp where s=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from sp_text_notes where s=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							if(!remove(oldp)){LOG_ERRs throw 0;}
							//? delete folder if there is no file under it and the DIR in memoizedcfg_t is not the folder name?
							end_tr();
							vb.clear();
						}
						break;
					case (unsigned char)'u':
						auto notep=e_p;
						auto oldp=notep;
						sqlite3_int64 id;
						memcpy(&id,rb+OFF,sizeof id);
						auto dnmlen=strlen((char *)rb+OFF+sizeof id);
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from sp_text_notes where d=? and f collate nocase =?"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF+sizeof id,dnmlen, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF+sizeof id+dnmlen+1,rl-OFF-sizeof id-dnmlen-1, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						switch(ib){
						case SQLITE_ROW:
							setresnotifyfe(LITERAL_COMMA_LEN("Filename is already used."));
							goto begin_db_l;
						case SQLITE_DONE:break;
						default:{LOG_Is(ib) throw 0;}
						}
						CATCH_SET_SUE_THROWs(;)
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select d,f from sp_text_notes where s=?"));)
						auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						switch(ib){
						case SQLITE_ROW:break;
						case SQLITE_DONE:
							setresnotifyfe(LITERAL_COMMA_LEN("\1Record missing. Might be already deleted."));
							goto begin_db_l;
						default:{LOG_Is(ib) throw 0;}
						}
						oldp/=(char *)sqlite3_column_text(sstmt.s.pstmt,0);
						oldp/=(char *)sqlite3_column_text(sstmt.s.pstmt,1);
						CATCH_SET_SUE_THROWs(;)
						begin_tr();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update sp_text_notes set d=?,f=? where s=?"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF+sizeof id,dnmlen, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF+sizeof id+dnmlen+1,rl-OFF-sizeof id-dnmlen-1, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_bind_int64(sstmt.s.pstmt, 3, id);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
						CATCH_SET_SUE_THROWs(;)
						notep/=(char *)rb+OFF+sizeof id;
						if(!exists(notep)){
							if(!create_directory(notep)){LOG_ERRs throw 0;}
						}
						notep.append((char *)rb+OFF+sizeof id+dnmlen+1,(char *)rb+rl);
						rename(oldp,notep);
						end_tr();
						vb.clear();
						break;
					}
				}
				break;
			case wordsense:{
					switch(rb[1]){
					case (unsigned char)'d':{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_singleargp where s=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from sp_essenttext where g=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							end_tr();
							vb.clear();
						}
						break;
					case (unsigned char)'S':{
							vb.clear();
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select t from sp_essenttext where g=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							for(;;){
								ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
										ib=sqlite3_column_bytes(sstmt.s.pstmt,0);
										resize_write_u32le(vb,ib);
										auto pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,0);
										vb.insert(vb.end(),pcc,pcc+ib);
									}
									continue;
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					case (unsigned char)'s':{
							THROW_IF_INVALID_REQUESTs(rl-OFF>0)
							int ib;
							vb.clear();
#					define SELECTGROUPNTEXTs \
								ib=sqlite3_bind_text(sstmt.s.pstmt, 1, uiformkbindval_tem.data(),uiformkbindval_tem.size(), SQLITE_TRANSIENT);\
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
								sqlite3_int64 previd=SENSEID_NOT_REAL;\
								for(;;){\
									ib=sqlite3_step(sstmt.s.pstmt);\
									switch(ib){\
									case SQLITE_ROW:{\
										auto gid=sqlite3_column_int64(sstmt.s.pstmt, 0);\
										if(SENSEID_NOT_REAL==previd){\
											vb.insert(vb.end(),(unsigned char *)&gid,(unsigned char *)(&gid+1));\
										}else if(gid!=previd){\
											vb.resize(vb.size()+4);\
											write_u32le(vb.data()+vb.size()-4,-1);\
											vb.insert(vb.end(),(unsigned char *)&gid,(unsigned char *)(&gid+1));\
										}\
										previd=gid;\
										ib=sqlite3_column_bytes(sstmt.s.pstmt,1);\
										vb.resize(vb.size()+4);\
										write_u32le(vb.data()+vb.size()-4,ib);\
										auto pcc=(const unsigned char *)sqlite3_column_blob(sstmt.s.pstmt,1);\
										vb.insert(vb.end(),pcc,pcc+ib);\
										continue;\
									}\
									case SQLITE_DONE:\
										break;\
									default:{LOG_Is(ib) throw 0;}\
									}\
									break;\
								}\
								CATCH_SET_SUE_THROWs(;)
							MAKEBINDVALTOFINDSTRWITHLIKEu(uiformkbindval_,(char *)rb+OFF,rl-OFF,{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from sp_essenttext where g in(select g from sp_essenttext where g in(select s from in_singleargp where v=" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ") and t like ?)order by g"));)//todo use MNUMOFRECORDS, which requires extra handling (omit last group bc you don't know if it includes all rows of that group)
								SELECTGROUPNTEXTs
							},{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from sp_essenttext where g in(select g from sp_essenttext where g in(select s from in_singleargp where v=" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ") and t like ? escape ?)order by g"));)
								ib=sqlite3_bind_text(sstmt.s.pstmt, 2, "`",1, SQLITE_TRANSIENT);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								SELECTGROUPNTEXTs
							})
						}
						break;
					case (unsigned char)'u':{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from sp_essenttext where g=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Record missing. Might be already deleted."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(dtor_delete_from_tmptab_b0 dtor_delete_from_tmptab_b0_;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into tmptab_b0 values(?)"));)
							decltype(rl) off=OFF+sizeof id;
							for(;;){
								auto blen=strlen((char *)rb+off);
								auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+off,blen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								off+=blen+1;
								if(off==rl) break;
								sqlite3_reset(sstmt.s.pstmt);
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_b0 group by z having count(*)>1"));)
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Duplicate words in a group is found."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from sp_essenttext where g=? and t not in tmptab_b0"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into sp_essenttext select ?1,z from tmptab_b0 where z not in(select t from sp_essenttext where g=?1)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							end_tr();
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}
						break;
					case (unsigned char)'i':{
							INIT_TRYs(dtor_delete_from_tmptab_b0 dtor_delete_from_tmptab_b0_;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into tmptab_b0 values(?)"));)
							decltype(rl) off=OFF;
							for(;;){
								auto blen=strlen((char *)rb+off);
								auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, rb+off,blen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								off+=blen+1;
								if(off==rl) break;
								sqlite3_reset(sstmt.s.pstmt);
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from tmptab_b0 group by z having count(*)>1"));)
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Duplicate words in a group is found."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							auto id=ret_avail_globalid();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into sp_essenttext select ?,z from tmptab_b0"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargp values(" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ",?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							update_avail_sense_id(); end_tr();
							vb.assign(1,0); vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					}
				}
				break;
			case 70:{//audio
				}
			case 71:{//video
				}
			case 72:{//software
				}
			case 73:{//diction (both thesaurus and dictionary)
				}
			case 128:{//generic search for anything//note cnf or dnf that is always true is the real implementation, but here only simplest entailment is implemented (entailment is the fake implementation, entailment doesn't really exist, long live normal forms)
					//undone when you receive the query from client here, do you need it transformed to Normal Form already?
					//probably no need to transform, it should be done at server side eventually anyway
					//e.g. A v (B^C) you do all the traversal at serverside
				}
				break;
			case 129:{//sql
					switch(rb[1]){
					case (unsigned char)'u'://update ? set (?=?,)+ where ??
						break;
					}
					//
				}
				break;
			case specialized_sql:{
					switch(rb[1]){
					case 0:
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update memoizedcfg_t set v=? where k='TEXTNOTE_DIR'"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						//ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, LITERAL_COMMA_LEN("TEXTNOTE_DIR"), SQLITE_STATIC);
						//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
						CATCH_SET_SUE_THROWs(;)
						vb.clear();
						break;
					case 1:{
							char *zErrMsg=nullptr;
							begin_tr();
							auto ib=sqlite3_exec(databaseA,"update memoizedcfg_t set k='TEXTNOTE_EDITOR' where k='TEXTNOTE_MAIN_EDITOR'", NULL, NULL, &zErrMsg);
							if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
							//INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update memoizedcfg_t set k='TEXTNOTE_EDITOR' where k='TEXTNOTE_MAIN_EDITOR'"));)
							//auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("TEXTNOTE_EDITOR"), SQLITE_STATIC);
							//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							//ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, LITERAL_COMMA_LEN("TEXTNOTE_MAIN_EDITOR"), SQLITE_STATIC);
							//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							//ib=sqlite3_step(sstmt.s.pstmt);
							//if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							//CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update memoizedcfg_t set k='TEXTNOTE_MAIN_EDITOR' where v=? and k='TEXTNOTE_EDITOR'"));)
							//auto ib=sqlite3_bind_blob(sstmt.s.pstmt, 1, LITERAL_COMMA_LEN("TEXTNOTE_MAIN_EDITOR"), SQLITE_STATIC);
							//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							//ib=sqlite3_bind_blob(sstmt.s.pstmt, 3, LITERAL_COMMA_LEN("TEXTNOTE_EDITOR"), SQLITE_STATIC);
							//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							if(sqlite3_changes(databaseA)==1){
								end_tr();
								vb.clear();
							}else{
								rb_tr();
								setresnotifyfe(LITERAL_COMMA_LEN("Failed to set main editor (might be deleted already)."));
								goto begin_db_l;
							}
						}
						break;
					case 2:
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from memoizedcfg_t where k in('TEXTNOTE_MAIN_EDITOR','TEXTNOTE_EDITOR')and v=?"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						switch(ib){
						case SQLITE_ROW:
							setresnotifyfe("",1);
							goto begin_db_l;
						case SQLITE_DONE:
							vb.clear();
							break;
						default:{LOG_Is(ib) throw 0;}
						}
						CATCH_SET_SUE_THROWs(;)
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into memoizedcfg_t values(?,case (select count(*)from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR')when 0 then 'TEXTNOTE_MAIN_EDITOR' else 'TEXTNOTE_EDITOR' end)"));)
						auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						//ib=sqlite3_bind_blob(sstmt.s.pstmt, 2, LITERAL_COMMA_LEN("TEXTNOTE_EDITOR"), SQLITE_STATIC);
						//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
						CATCH_SET_SUE_THROWs(;)
						break;
					case 3:{//todo the logic for deletion of cmd is not exactly correct. need to consider multiple UI deletion of same cmd?
							vb.clear();
							int ib;
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR' and v=?"));)
							ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							CATCH_SET_SUE_THROWs(;)
							switch(ib){
								case SQLITE_ROW:{
									char *zErrMsg=nullptr;
									begin_tr();
									ib=sqlite3_exec(databaseA,"delete from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR'", NULL, NULL, &zErrMsg);
									if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
									ib=sqlite3_exec(databaseA,"update memoizedcfg_t set k='TEXTNOTE_MAIN_EDITOR' where(v,k)=(select v,k from memoizedcfg_t where k='TEXTNOTE_EDITOR' limit 1)", NULL, NULL, &zErrMsg);
									if(SQLITE_OK!=ib){LOG_I_CIFNOTNULLSQLITEFREEs(ib,zErrMsg) throw 0;}
									end_tr();
									INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from memoizedcfg_t where k='TEXTNOTE_MAIN_EDITOR'"));)
									ib=sqlite3_step(sstmt.s.pstmt);
									switch(ib){
									case SQLITE_ROW:{
										ib=sqlite3_column_bytes(sstmt.s.pstmt,0);
										auto pcc=sqlite3_column_text(sstmt.s.pstmt,0);
										vb.insert(vb.end(),pcc,pcc+ib);
										break;
									}
									case SQLITE_DONE:
										break;
									default:{LOG_Is(ib) throw 0;}
									}
									CATCH_SET_SUE_THROWs(;)
									break;
								}
								case SQLITE_DONE:
									INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from memoizedcfg_t where v=? and k='TEXTNOTE_EDITOR'"));)
									ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+2,rl-2, SQLITE_TRANSIENT);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									ib=sqlite3_step(sstmt.s.pstmt);
									if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
									CATCH_SET_SUE_THROWs(;)
									break;
								default:{LOG_Is(ib) throw 0;}
							}
						}break;
					case 4:
						vb.clear();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_singleargv where v not in(select a from in_entailment) limit " XSTR(MNUMOFRECORDS)));)
						for(;;){
							auto ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
								auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
								vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
								ib=sqlite3_column_bytes(sstmt.s.pstmt,1);
								resize_write_u32le(vb,ib);
								auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
								vb.insert(vb.end(),pcc,pcc+ib);
								continue;
							}
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					case 5:{
							int ib;
							vb.clear();
#					define SELECTVERBLIKEs \
								ib=sqlite3_bind_text(sstmt.s.pstmt, 1, uiformkbindval_tem.data(),uiformkbindval_tem.size(), SQLITE_TRANSIENT);\
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
								for(;;){\
									ib=sqlite3_step(sstmt.s.pstmt);\
									switch(ib){\
									case SQLITE_ROW:{\
										auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);\
										vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));\
										ib=sqlite3_column_bytes(sstmt.s.pstmt,1);\
										vb.resize(vb.size()+4);\
										write_u32le(vb.data()+vb.size()-4,ib);\
										auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);\
										vb.insert(vb.end(),pcc,pcc+ib);\
										continue;\
									}\
									case SQLITE_DONE:\
										break;\
									default:{LOG_Is(ib) throw 0;}\
									}\
									break;\
								}\
								CATCH_SET_SUE_THROWs(;)
							MAKEBINDVALTOFINDSTRWITHLIKEu(uiformkbindval_,(char *)rb+OFF,rl-OFF,{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_singleargv where d like ? limit " XSTR(MNUMOFRECORDS)));)
								SELECTVERBLIKEs
							},{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_singleargv where d like ? escape ? limit " XSTR(MNUMOFRECORDS)));)
								ib=sqlite3_bind_text(sstmt.s.pstmt, 2, "`",1, SQLITE_TRANSIENT);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								SELECTVERBLIKEs
							})
						}
						break;
					case 10:{
							int ib;
							vb.clear();
							MAKEBINDVALTOFINDSTRWITHLIKEu(uiformkbindval_,(char *)rb+OFF,rl-OFF,{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from(select * from tmptab_v union all select * from in_singleargv)where d like ? limit " XSTR(MNUMOFRECORDS)));)
								SELECTVERBLIKEs
							},{
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from(select * from tmptab_v union all select * from in_singleargv)where d like ? escape ? limit " XSTR(MNUMOFRECORDS)));)
								ib=sqlite3_bind_text(sstmt.s.pstmt, 2, "`",1, SQLITE_TRANSIENT);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								SELECTVERBLIKEs
							})
						}
						break;
					case 12:{
							THROW_IF_INVALID_REQUESTs(rl-OFF)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from(select * from tmptab_v union all select * from in_singleargv)where d=?"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Duplicate description found in db."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							auto newid=ret_avail_globalid();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_singleargv values(?,?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, newid);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_text(sstmt.s.pstmt, 2, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							update_avail_sense_id(); end_tr();
							vb.assign(1,0); vb.insert(vb.end(),(unsigned char *)&newid,(unsigned char *)(&newid+1));
						}
						break;
					case 21:{
							vb.clear();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_singleargv where instr(lower(d),lower(?)) order by length(d) limit 32"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,rl-OFF, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							for(;;){
								ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
									auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
									vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
									ib=sqlite3_column_bytes(sstmt.s.pstmt,1);
									resize_write_u32le(vb,ib);
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
									vb.insert(vb.end(),pcc,pcc+ib);
									continue;
								}
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
						}break;
					case 6:{
							if(rl==OFF){
								setresnotifyfe(LITERAL_COMMA_LEN("Description cannot be empty."));
								goto begin_db_l;
							}
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v from(select * from tmptab_v union all select * from in_singleargv)where d=?"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF+sizeof id,rl-OFF-sizeof id, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								if(id==sqlite3_column_int64(sstmt.s.pstmt, 0))//no need to select 2nd row bc desc should unique, unless db is corrupt
									setresnotifyfe(LITERAL_COMMA_LEN("No change in description."));
								else
									setresnotifyfe(LITERAL_COMMA_LEN("Duplicate description found in db."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("update in_singleargv set d=? where v=?"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF+sizeof id,rl-OFF-sizeof id, SQLITE_TRANSIENT);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt, 2, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							if(sqlite3_changes(databaseA)==1){
								vb.clear();
							}else{
								setresnotifyfe(LITERAL_COMMA_LEN("Update failed (might be deleted already)."));
								goto begin_db_l;
							}
						}
						break;
					case 7:{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							vb.clear();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v,d from in_entailment join in_singleargv on c=v where a=? limit " XSTR(MNUMOFRECORDS)));)
							#define BINDIDINSINTTEXTCSST \
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);\
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}\
							for(;;){\
								ib=sqlite3_step(sstmt.s.pstmt);\
								switch(ib){\
								case SQLITE_ROW:{\
										auto vid=sqlite3_column_int64(sstmt.s.pstmt, 0);\
										vb.insert(vb.end(),(unsigned char *)&vid,(unsigned char *)(&vid+1));\
										auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);\
										vb.insert(vb.end(),pcc,pcc+sqlite3_column_bytes(sstmt.s.pstmt,1)+1);\
									}\
									continue;\
								case SQLITE_DONE:\
									break;\
								default:{LOG_Is(ib) throw 0;}\
								}\
								break;\
							}\
							CATCH_SET_SUE_THROWs(;)
							BINDIDINSINTTEXTCSST
							vb.resize(vb.size()+sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v,d from in_entailment join in_singleargv on a=v where c=? limit " XSTR(MNUMOFRECORDS)));)
							BINDIDINSINTTEXTCSST
						}
						break;
					case 13:{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							vb.clear();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v,d from in_entailment join in_singleargv on c=v where a=? limit " XSTR(MNUMOFRECORDS)));)
							BINDIDINSINTTEXTCSST
						}break;
					case 14:{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							vb.clear();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select v,d from in_entailment join in_singleargv on a=v where c=? limit " XSTR(MNUMOFRECORDS)));)
							BINDIDINSINTTEXTCSST
						}break;
					case 8:{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							vb.clear();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select g,d from in_mutexgroup join in_singleargv on in_singleargv.v=in_mutexgroup.v where in_mutexgroup.v<>?1 and g in(select g from in_mutexgroup where v=?1) order by g"));)//not using limit bc mutex groups are not supposed to be so numerous
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							sqlite3_int64 previd=SENSEID_NOT_REAL;
							for(;;){
								ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
										auto gid=sqlite3_column_int64(sstmt.s.pstmt, 0);
										if(SENSEID_NOT_REAL==previd){
											vb.insert(vb.end(),(unsigned char *)&gid,(unsigned char *)(&gid+1));
										}else if(gid==previd){
											vb.push_back(1);
										}else{
											vb.push_back(0);
											vb.insert(vb.end(),(unsigned char *)&gid,(unsigned char *)(&gid+1));
										}
										previd=gid;
										auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
										vb.insert(vb.end(),pcc,pcc+sqlite3_column_bytes(sstmt.s.pstmt,1)+1);
									}
									continue;
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					case 9:{
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from in_mutexgroup where v=?1 union all select 0 from in_entailment where c=?1 union all select 0 from in_singleargp where v=?1"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Verb cannot be deleted if it's used by any record, entailed by any verb, or in any mutually exclusive group."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_entailment where a=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_singleargv where v=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							end_tr();
							vb.clear();
						}
						break;
					case 11:{
							vector<char *> texttobebound;
							vector<sqlite3_int64> int64tobebound;
							vector<bool> tyofbinding;
							sqlite3_int64 indoftextse=0;
							vector<char> sql;{
								insert_end_literal(sql,"select * from(select " XSTR(bookmark) " t,i from sp_urlbookmar union all select " XSTR(text_note) ",s from sp_text_notes union all select " XSTR(wordsense) ",s from in_singleargp where v=" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ")where");
							}
							long pcount=0;
							unsigned long off=OFF;
							INIT_TRYs(dtor_delete_from_tmptab_ii0 dtor_delete_from_tmptab_ii0_;)
							regex sepregex{"\\)*(or|and|)\\(*"};
							for(;;){
								cmatch cmbuf;
								THROW_IF_INVALID_REQUESTs(regex_match((char *)rb+off,cmbuf,sepregex))//need to cast to char ???
								auto slen=cmbuf.length();
								auto sepp=cmbuf.position(1);
								auto sepl=cmbuf.length(1);
								pcount-=sepp;
								if(pcount<0) {setresnotifyfe(LITERAL_COMMA_LEN("\1Parentheses do not match.")); goto begin_db_l;}
								pcount+=slen-sepl-sepp;
								if(off==OFF){
										sql.insert(sql.end(),slen-sepl-sepp,'(');
								}else{
									if(sepl){
										if(!sepp)sql.push_back(' ');
										sql.insert(sql.end(),rb+off,rb+off+slen);
									}else{
										sql.insert(sql.end(),sepp,')');
										insert_end_literal(sql," and");
										sql.insert(sql.end(),slen-sepl-sepp,'(');
									}
								}
								off+=slen+1;
								if(off==rl){
									if(pcount){setresnotifyfe(LITERAL_COMMA_LEN("\1Parentheses do not match.")); goto begin_db_l;}
									if(sql.back()=='r')sql.resize(sql.size()-2);//remove "or"
									else sql.resize(sql.size()-3);//remove "and"
									insert_end_literal_with_nul(sql,"limit " XSTR(MNUMOFRECORDS));
									break;
								}
								sqlite3_int64 id;
								memcpy(&id,rb+off+1,sizeof id);
								switch(id){
								case IS_A_NATURAL_LANG_WORD_SENSE:
									if(rb[off]){auto &lit=" t=" XSTR(wordsense); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit=" t<>" XSTR(wordsense); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									break;
								case VERB_CONTAINS_TEXT:{
										if(rb[off]){auto &lit=" i in(select g from(select * from sp_essenttext union all select i,d from sp_urlbookmar union all select i,u from sp_urlbookmar)where instr(lower(t),lower(?))union all select y from tmptab_ii0 where z="; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
										else{auto &lit=" i not in(select g from(select * from sp_essenttext union all select i,d from sp_urlbookmar union all select i,u from sp_urlbookmar)where instr(lower(t),lower(?))union all select y from tmptab_ii0 where z="; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
										auto firstcolv=to_string(indoftextse);
										sql.insert(sql.end(),firstcolv.data(),firstcolv.data()+firstcolv.size());
										sql.push_back(')');
										off+=1+sizeof id;
										int len=strlen((char *)rb+off);//int overflow could occur
										addtextnoteseresultstotmp(firstcolv,rb+off,len);
										++indoftextse;
										texttobebound.push_back((char *)rb+off);
										tyofbinding.push_back(true);
										off+=len+1;
									}
									break;
								case ANY_USER_DEFINED_VERB:
									if(rb[off]){auto &lit=" i in(select s from in_singleargp where v<>" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ")"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit=" i not in(select s from in_singleargp where v<>" XSTR(IS_A_NATURAL_LANG_WORD_SENSE) ")"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									break;
								case IS_A_TEXT_NOTE:
									if(rb[off]){auto &lit=" t=" XSTR(text_note); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit=" t<>" XSTR(text_note); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									break;
								case IS_A_BOOKMARK:
									if(rb[off]){auto &lit=" t=" XSTR(bookmark); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit=" t<>" XSTR(bookmark); sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									break;
								case VERB_URL_CONTAINS_TEXT:
									//note using XSTR(bookmark) here is only for performance reason, they are not really necessary
									if(rb[off]){auto &lit="(t=" XSTR(bookmark) " and i in(select i from sp_urlbookmar where instr(lower(u),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit="(t<>" XSTR(bookmark) " or i not in(select i from sp_urlbookmar where instr(lower(u),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									texttobebound.push_back((char *)rb+off);
									off+=strlen((char *)rb+off)+1;
									tyofbinding.push_back(true);
									break;
								case VERB_FILENAME_CONTAINS_TEXT:
									if(rb[off]){auto &lit="(t=" XSTR(text_note) " and i in(select s from sp_text_notes where instr(lower(f),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit="(t<>" XSTR(text_note) " or i not in(select s from sp_text_notes where instr(lower(f),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									texttobebound.push_back((char *)rb+off);
									off+=strlen((char *)rb+off)+1;
									tyofbinding.push_back(true);
									break;
								case VERB_DESC_CONTAINS_TEXT:
									if(rb[off]){auto &lit="(i in(select g from(select * from sp_essenttext union all select i,d from sp_urlbookmar)where instr(lower(t),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit="(i not in(select g from(select * from sp_essenttext union all select i,d from sp_urlbookmar)where instr(lower(t),lower(?))))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									texttobebound.push_back((char *)rb+off);
									off+=strlen((char *)rb+off)+1;
									tyofbinding.push_back(true);
									break;
								default:
									tyofbinding.push_back(false);
									int64tobebound.push_back(id);
									if(rb[off]){auto &lit=" i in(select s from in_singleargp where v in(with a_a(z)as(values(?)union all select a from in_entailment join a_a on c=z)select z from a_a))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									else{auto &lit=" i not in(select s from in_singleargp where v in(with a_a(z)as(values(?)union all select a from in_entailment join a_a on c=z)select z from a_a))"; sql.insert(sql.end(),lit,(const char *)(&lit+1)-1);}
									off+=1+sizeof id;
									break;
								}
							}
							INIT_TRYs(slw_prestmt sstmt(databaseA, sql.data(), sql.size());)
							decltype(int64tobebound)::size_type int64index=0;
							decltype(texttobebound)::size_type textindex=0;
							for(int bindindex=0;bindindex<tyofbinding.size();){
								if(tyofbinding[bindindex]){
									++bindindex;
									auto ib=sqlite3_bind_text(sstmt.s.pstmt,bindindex,texttobebound[textindex],-1,SQLITE_TRANSIENT_STATIC_BEF_FIN);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									//remove no need to discriminate blob and text
									//sqlite3_int64 id;
									//memcpy(&id,texttobebound[textindex]-sizeof id,sizeof id);
									//if(id==VERB_FILENAME_CONTAINS_TEXT){
									//	auto ib=sqlite3_bind_text(sstmt.s.pstmt,bindindex,texttobebound[textindex],-1,SQLITE_TRANSIENT_STATIC_BEF_FIN);
									//	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									//}else{
									//	auto ib=sqlite3_bind_blob(sstmt.s.pstmt,bindindex,texttobebound[textindex],strlen(texttobebound[textindex]),SQLITE_TRANSIENT_STATIC_BEF_FIN);
									//	if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									//}
									++textindex;
								}else{
									++bindindex;
									auto ib=sqlite3_bind_int64(sstmt.s.pstmt,bindindex,int64tobebound[int64index]);
									if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
									++int64index;
								}
							}
							vb.assign(1,0);
							INIT_TRYs(slw_prestmt tnstmt(databaseA, LITERAL_COMMA_SIZE("select d,f from sp_text_notes where s=?"));)
							INIT_TRYs(slw_prestmt ubstmt(databaseA, LITERAL_COMMA_SIZE("select u,d from sp_urlbookmar where i=?"));)
							INIT_TRYs(slw_prestmt etstmt(databaseA, LITERAL_COMMA_SIZE("select t from sp_essenttext where g=?"));)
							for(;;){
								auto ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
										unsigned char tcty=sqlite3_column_int64(sstmt.s.pstmt, 0);
										vb.push_back(tcty);
										auto id=sqlite3_column_int64(sstmt.s.pstmt, 1);
										vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
										switch(tcty){
										case bookmark:{
												sqlite3_reset(ubstmt.s.pstmt);
												ib=sqlite3_bind_int64(ubstmt.s.pstmt,1,id);
												if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
												ib=sqlite3_step(ubstmt.s.pstmt);
												if(SQLITE_ROW!=ib){LOG_Is(ib) throw 0;}
												ib=sqlite3_column_bytes(ubstmt.s.pstmt, 0);
												resize_write_u32le(vb,ib);
												auto pcc=(const unsigned char *)sqlite3_column_blob(ubstmt.s.pstmt,0);
												vb.insert(vb.end(),pcc,pcc+ib);
												ib=sqlite3_column_bytes(ubstmt.s.pstmt, 1);
												resize_write_u32le(vb,ib);
												pcc=(const unsigned char *)sqlite3_column_blob(ubstmt.s.pstmt,1);
												vb.insert(vb.end(),pcc,pcc+ib);
											}break;
										case text_note:{
												sqlite3_reset(tnstmt.s.pstmt);
												ib=sqlite3_bind_int64(tnstmt.s.pstmt,1,id);
												if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
												ib=sqlite3_step(tnstmt.s.pstmt);
												if(SQLITE_ROW!=ib){LOG_Is(ib) throw 0;}
												auto dirnm=sqlite3_column_text(tnstmt.s.pstmt,0);
												auto filenm=sqlite3_column_text(tnstmt.s.pstmt,1);
												vb.insert(vb.end(),dirnm,dirnm+sqlite3_column_bytes(tnstmt.s.pstmt,0));
												vb.push_back('/');
												vb.insert(vb.end(),filenm,filenm+sqlite3_column_bytes(tnstmt.s.pstmt,1)+1);
											}break;
										case wordsense:
											sqlite3_reset(etstmt.s.pstmt);
											ib=sqlite3_bind_int64(etstmt.s.pstmt,1,id);
											if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
											for(;;){
												vb.resize(vb.size()+4);
												ib=sqlite3_step(etstmt.s.pstmt);
												switch(ib){
												case SQLITE_ROW:{
														ib=sqlite3_column_bytes(etstmt.s.pstmt,0);
														write_u32le(vb.data()+vb.size()-4,ib);
														auto pcc=(const unsigned char *)sqlite3_column_blob(etstmt.s.pstmt,0);
														vb.insert(vb.end(),pcc,pcc+ib);
													}
													continue;
												case SQLITE_DONE:
													write_u32le(vb.data()+vb.size()-4,-1);
													break;
												default:{LOG_Is(ib) throw 0;}
												}
												break;
											}
											break;
										}
									}
									continue;
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					case 15:{
							sqlite3_int64 sibufa,sibufc;
							memcpy(&sibufa,rb+OFF,sizeof(sqlite3_int64));
							memcpy(&sibufc,rb+OFF+sizeof(sqlite3_int64),sizeof(sqlite3_int64));
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select 0 from in_singleargv where v=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Verb not found (might be already deleted)."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							sqlite3_reset(sstmt.s.pstmt);
							ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								break;
							case SQLITE_DONE:
								setresnotifyfe(LITERAL_COMMA_LEN("Verb not found (might be already deleted)."));
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(values(?)union select a from in_entailment join a_a on c=z),a_c(x)as(values(?)union select c from in_entailment join a_c on a=x)select * from in_entailment where a in a_a and c in a_c"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Failed: Tautology."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(values(?) union select c from in_entailment join a_a on a=z)select 0 from a_a where z=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Failed: Tautology."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							sqlite3_reset(sstmt.s.pstmt);
							ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Failed: Oxymoron."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(values(?)union select a from in_entailment join a_a on c=z),a_b(y)as(select z from a_a union select c from in_entailment join a_b on a=y),a_c(x)as(values(?)union select c from in_entailment join a_c on a=x)select * from (select * from in_mutexgroup where v in a_b)a_d join (select * from in_mutexgroup where v in a_c)a_e on a_d.g=a_e.g and a_d.v<>a_e.v"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Failed: Oxymoron."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(values(?)union select a from in_entailment join a_a on c=z),a_c(x)as(values(?)union select c from in_entailment join a_c on a=x)select * from(select * from in_singleargp where v in a_a)a_d join(select * from in_singleargp where v in a_c)a_e on a_d.s=a_e.s"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("Failed: Invalid State."));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_entailment values(?,?)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}
						break;
					case 16:{
							sqlite3_int64 sibufa,sibufc;
							memcpy(&sibufa,rb+OFF,sizeof(sqlite3_int64));
							memcpy(&sibufc,rb+OFF+sizeof(sqlite3_int64),sizeof(sqlite3_int64));
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_entailment where a=? and c=?"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt,1,sibufa);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_bind_int64(sstmt.s.pstmt,2,sibufc);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}
						break;
					case 17:{
							THROW_IF_INVALID_REQUESTs(rl!=OFF)
							INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)
							TEMPRECEIVESINGLEARGPROPSWITHTMPTABANDCHKs(OFF)
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}
						break;
					case 18:{
							THROW_IF_INVALID_REQUESTs(rl!=OFF)
							INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)
							TEMPRECEIVESINGLEARGPROPSWITHTMPTABANDCHKs(OFF)
							vb.assign(1,0);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("with a_a(z)as(select z from tmptab_i0 union all select c from in_entailment join a_a on a=z)select * from in_singleargv where v not in a_a and v not in(select a from in_entailment where c not in a_a) and v not in(select v from in_mutexgroup where g in(select g from in_mutexgroup where v in a_a)) and v not in(select a from in_entailment where c in tmptab_i0)"));)
							for(;;){
								auto ib=sqlite3_step(sstmt.s.pstmt);
								switch(ib){
								case SQLITE_ROW:{
									auto id=sqlite3_column_int64(sstmt.s.pstmt, 0);
									vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
									ib=sqlite3_column_bytes(sstmt.s.pstmt,1);
									resize_write_u32le(vb,ib);
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
									vb.insert(vb.end(),pcc,pcc+ib);
									continue;
								}
								case SQLITE_DONE:
									break;
								default:{LOG_Is(ib) throw 0;}
								}
								break;
							}
							CATCH_SET_SUE_THROWs(;)
							CATCH_SET_SUE_THROWs(;)
						}
						break;
					case 19:{
							sqlite3_int64 sens;
							memcpy(&sens,rb+OFF,sizeof(sqlite3_int64));
							if(rl==OFF+sizeof(sqlite3_int64)){
								DELETEALLSINGLEARGPROPSOFSENSs(sens)
							}else{
								RECEIVESINGLEARGPROPSCHKANDINSERTs(OFF+sizeof(sqlite3_int64),sens,)
							}
							vb.clear();
						}
						break;
					case 20:{
							sqlite3_int64 sens;
							memcpy(&sens,rb+OFF,sizeof(sqlite3_int64));
							vb.clear();
							selectprops(vb,sens);
						}break;
					case 22:{
							sqlite3_int64 id;
							auto slen=strlen((char *)rb+OFF);
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_singleargv where d=?"));)
							auto ib=sqlite3_bind_text(sstmt.s.pstmt, 1, (char *)rb+OFF,slen, SQLITE_TRANSIENT_STATIC_BEF_FIN);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								vb.assign(1, 0);
								id=sqlite3_column_int64(sstmt.s.pstmt, 0);
								vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
								break;
							case SQLITE_DONE:
								setresnotifyfe(VALID_PH_PTR,0);
								goto begin_db_l;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into tmptab_i0 values(?)"));)
							for(decltype(rl) off=OFF+slen+1;;){
								auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								if(off==rl) break;
								sqlite3_reset(sstmt.s.pstmt);
								memcpy(&id,rb+off,sizeof id);
								off+=sizeof id;
							}
							CATCH_SET_SUE_THROWs(;)
							TEMPSINGLEARGPROPSWITHTMPTABANDCHKs
							CATCH_SET_SUE_THROWs(;)
						}break;
					case 23:{//note that tautology is only checked when saving
							THROW_IF_INVALID_REQUESTs(rl>OFF+sizeof(sqlite3_int64))
							INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)
							TEMPRECEIVESINGLEARGVERBSWITHTMPTABANDCHKs(OFF)
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}break;
					case 24:{
							THROW_IF_INVALID_REQUESTs(rl>OFF+sizeof(sqlite3_int64)*2)
							INIT_TRYs(dtor_delete_from_tmptab_i0 dtor_delete_from_tmptab_i0_;)
							sqlite3_int64 id;
							memcpy(&id,rb+OFF,sizeof id);
							TEMPRECEIVESINGLEARGVERBSWITHTMPTABANDCHKs(OFF+sizeof(sqlite3_int64))
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from in_mutexgroup where g<>? and g not in(select g from in_mutexgroup where v not in tmptab_i0)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Tautology"));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select g from in_mutexgroup where g<>? and v in tmptab_i0 group by g having count(*)=(select count(*) from tmptab_i0)"));)
							auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
							if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:
								setresnotifyfe(LITERAL_COMMA_LEN("\1Tautology"));
								goto begin_db_l;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							CATCH_SET_SUE_THROWs(;)
							begin_tr();
							if(id){
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_mutexgroup where g=? and v not in tmptab_i0"));)
								auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								CATCH_SET_SUE_THROWs(;)
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_mutexgroup select ?1,z from tmptab_i0 where z not in(select v from in_mutexgroup where g=?1)"));)
								auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								CATCH_SET_SUE_THROWs(;)
							}else{
								id=ret_avail_globalid();
								INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("insert into in_mutexgroup select ?,z from tmptab_i0"));)
								auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
								if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
								ib=sqlite3_step(sstmt.s.pstmt);
								if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
								CATCH_SET_SUE_THROWs(;)
								update_avail_sense_id();
							}
							end_tr();
							CATCH_SET_SUE_THROWs(;)
							vb.clear();
						}break;
					case 25:
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("delete from in_mutexgroup where g=?"));)
						sqlite3_int64 id;
						memcpy(&id,rb+OFF,sizeof id);
						auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						ib=sqlite3_step(sstmt.s.pstmt);
						if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
						CATCH_SET_SUE_THROWs(;)
						vb.clear();
						break;
					case 26:
						vb.clear();
						INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select in_mutexgroup.v,d from in_mutexgroup join in_singleargv on in_singleargv.v=in_mutexgroup.v where g=?"));)
						sqlite3_int64 id;
						memcpy(&id,rb+OFF,sizeof id);
						auto ib=sqlite3_bind_int64(sstmt.s.pstmt, 1, id);
						if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
						for(;;){
							ib=sqlite3_step(sstmt.s.pstmt);
							switch(ib){
							case SQLITE_ROW:{
									id=sqlite3_column_int64(sstmt.s.pstmt, 0);
									vb.insert(vb.end(),(unsigned char *)&id,(unsigned char *)(&id+1));
									ib=sqlite3_column_bytes(sstmt.s.pstmt,1);
									resize_write_u32le(vb,ib);
									auto pcc=sqlite3_column_text(sstmt.s.pstmt,1);
									vb.insert(vb.end(),pcc,pcc+ib);
								}continue;
							case SQLITE_DONE:
								break;
							default:{LOG_Is(ib) throw 0;}
							}
							break;
						}
						CATCH_SET_SUE_THROWs(;)
						break;
					default:
						LOG_ERRs throw 0;
					}
				}
				break;
			case 170:{
					switch(rb[1]){
					case 0:{
							auto ffnmlen=strlen((char *)rb+OFF);
							auto fullfilenm{u8path((char *)rb+OFF)};//optimize use InputIt
							if(exists(fullfilenm)){
								setresnotifyfe(LITERAL_COMMA_LEN("Filename exists"));
								goto begin_db_l;
							}
							auto lengthuntilcontent=OFF+ffnmlen+1;
							ofstream ofstre_(fullfilenm,ios_base::binary);
							if(!ofstre_){setresnotifyfe(LITERAL_COMMA_LEN("ctor err"));goto begin_db_l;}
							ofstre_.write((char *)rb+lengthuntilcontent,rl-lengthuntilcontent);
							if(!ofstre_){setresnotifyfe(LITERAL_COMMA_LEN("write err"));goto begin_db_l;}
							ofstre_.close();//dtor doesn't throw even if failbit is set
							if(!ofstre_){setresnotifyfe(LITERAL_COMMA_LEN("dtor err"));goto begin_db_l;}
							vb.clear();
							break;
						}
					default:{
							LOG_ERRs throw 0;
							break;
						}
					}
				}
				break;
			default:
				LOG_ERRs throw 0;
			}
			setresnotifyfe(vb.data(),vb.size());
			//setresnotifyfe(resfs,resfslen);
		}
		//CATCH_SET_SUE_THROWs(;)
		//CATCH_SET_SUE_THROWs(;)
		CATCH_SET_SUE_THROWs(;)
		after_db_close:
		clog<<"<\n"<<flush;
		//note push event is not necessary when the shutdown comes from SDL_QUIT, but doing so doesn't hurt, then let it be
		//note push event should be done before setting reslen, in case the shutdown comes from SDL_QUIT, to make sure SDL_PushEvent is called before SDL_Quit()
		sdl_pushevent_userevent();
		setresnotifyfe("",RESLEN_DB_ALREADY_SHUT_DOWN);
		return;
	}catch(const std::exception &e){
		LOG_E_NOEXCEPTs(e)
	}catch(...){
		LOG_ERR_NOEXCEPTs
	}
	try{
		sdl_pushevent_userevent();
		{//after error still notify about dbready/newreq
			lock_guard<mutex> lg_{dbm};
			lock_guard<mutex> lgp{fem};
			rl=0;
			dbready=DB_INIT_ERR;//for dbready
			reslen=RESLEN_ERR;//for newreq
		}
		fecv.notify_all();
	}catch(const std::exception &e){
		LOG_E_NOEXCEPTs(e)
	}catch(...){
		LOG_ERR_NOEXCEPTs
	}
}

void reqshutdowndb(void){
	unsigned char ucbuf=shutdown;
	void *rph;
	int lph;
	newreq(1,&ucbuf,&rph,&lph);
	if(lph!=RESLEN_DB_ALREADY_SHUT_DOWN){
		//undone display something and change title to notify about "something went wrong"
	}
}

void reqshutdowndb_noexcept(void){
	try{reqshutdowndb();}catch(...){}
}

struct dbthre{
	thread stdt{db};
	~dbthre(){
		if(stdt.joinable()){
			try{//this is only reachable when stack unwinding is in progress
				reqshutdowndb();
				stdt.join();
			}catch(...){}
		}
	}
};


#include <sdlminwrapper.h>

unsigned char freopen_out_err(char *efromgo){//undone change function name to freopenouterr_createdirifnotpresent
	try{
		lock_guard<mutex> lgp{dbm};
		//char *e_customizable=getenv("ECSBAS_DATA_DIR");//you have only one env var to get, no need to avoid race
		//if(!e_customizable){
		//	clog<<"Environment variable 'ECSBAS_DATA_DIR' is not set.";
		//	//STD_CLOG_TIME_FILE_FUNC_LINE_FLUSH;
		//	return 1;
		//}
		auto cclog{u8path(efromgo)};
		e_customizable_p=cclog;
		cclog/="ecsbas";
		e_p=cclog;
		auto ecsb=cclog;
		cclog/="_log";
		if(!is_directory(symlink_status(cclog))){
			if(!create_directories(cclog)){
				LOG_ERRs
				return 1;
			}
		}
		auto ccout=cclog;
		cclog/="clog";
		ccout/="cout";
		if(!FREOPEN_WITH_NATIVE_FNM_ENC(cclog.c_str(),"ab",stderr)){//undone is "ab" used here okay? at least it seems okay on linux and windows
			STD_CLOG_TIME_FILE_FUNC_LINE_FLUSH;
			return 1;
		}
		if(!FREOPEN_WITH_NATIVE_FNM_ENC(ccout.c_str(),"ab",stdout)){
			STD_CLOG_TIME_FILE_FUNC_LINE_FLUSH;
			return 1;
		}
		ecsb/="_db";
		if(!is_directory(symlink_status(ecsb))){
			if(!create_directory(ecsb)){
				LOG_ERRs
				return 1;
			}
		}
		ecsb.replace_filename("notes");
		if(!is_directory(symlink_status(ecsb))){
			if(!create_directory(ecsb)){
				LOG_ERRs
				return 1;
			}
		}
		ecsb.replace_filename("_bookmark");
		if(!is_directory(symlink_status(ecsb))){
			if(!create_directory(ecsb)){
				LOG_ERRs
				return 1;
			}
		}
		clog<<">\n"<<flush;
		return 0;
		//no need to fclose stderr and stdout at exit (besides, all streams should be closed when processe ends anyway)
	}catch(const std::exception &e){
		STD_CLOG_TIME_FILE_FUNC_LINE_EX_FLUSH_NOEXCEPTs(e)
	}catch(...){
		STD_CLOG_TIME_FILE_FUNC_LINE_FLUSH_NOEXCEPTs
	}
	return 1;
}

void showwin(void){//noexcept
	try{
		sdl sdl_ph{SDL_INIT_VIDEO};
		window wind{"ECSBAS",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,660,195,0};//undone try increasing height? does that fix the issue of partial rendering on linux?
		dbthre dbtph;
		SDL_Surface *surface;
		//Uint16 pxint[16*16] = {
		//};
		//surface = SDL_CreateRGBSurfaceFrom(pxint,16,16,16,16*2,0x0f00,0x00f0,0x000f,0xf000);
		surface=SDL_LoadBMP("icon.bmp");
		if(!surface){LOG_ERR_TRY_GET_SDL_ERRs throw 0;}
		SDL_SetWindowIcon(wind.sdlw, surface);
		SDL_FreeSurface(surface);
		SDL_Surface *wsur=SDL_GetWindowSurface(wind.sdlw);
		if(!wsur){LOG_ERR_TRY_GET_SDL_ERRs throw 0;}
		surface=SDL_LoadBMP("surf.bmp");
		if(!surface){LOG_ERR_TRY_GET_SDL_ERRs throw 0;}
		auto rBlitSurface=SDL_BlitSurface(surface,NULL,wsur,NULL);
		SDL_FreeSurface(surface);
		if(rBlitSurface){LOG_ERR_TRY_GET_SDL_ERRs throw 0;}
		if(SDL_UpdateWindowSurface(wind.sdlw)){LOG_ERR_TRY_GET_SDL_ERRs throw 0;}

		auto ucbuf = waituntildbisready();
		if(ucbuf!=DB_INIT_READY){
			//undone display something on window?
			return;
		}

		SDL_Event event;
		while(SDL_WaitEvent(&event)){
			switch(event.type){
			//QUIT from window close button
			case SDL_QUIT:{
					reqshutdowndb();
					dbtph.stdt.join();
					return;
				}
				break;
			//db is already closed
			case SDL_USEREVENT:{
					dbtph.stdt.join();
					//?if(reslen<=RESLEN_ERR) show 'some error occurred'?.

					//if(event.user.code){//?error exit? could be error derived from go code or c++ db thread code?
					//}else{//?normal exit?
					//}//todo add config option about disabling exit from remote client?
					return;
				}
				break;
			}
		}
		LOG_ERR_TRY_GET_SDL_ERRs
		reqshutdowndb();
		dbtph.stdt.join();
	}catch(const std::exception &e){
		LOG_E_NOEXCEPTs(e)
	}catch(...){
		LOG_ERR_NOEXCEPTs
	}
}



