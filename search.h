
template<class T1,class T2,class T3>
void addtextnoteseresultstotmp(const T1 &firstcolv,const T2 str,T3 len){
	THROW_IF_INVALID_REQUESTs(len>1)
	THROW_IF_INVALID_REQUESTs(CHBUFSI>=2*(len-1))
	unique_ptr<char[]> sybuf(new char[SYNOPSISLEN]);
	unique_ptr<char[]> chbuf(new char[CHBUFSI]);
	string sql(LITERAL_COMMA_LEN("insert into tmptab_ii0 values("));
	sql+=firstcolv;
	sql.append(LITERAL_COMMA_LEN(",?)"));
	INIT_TRYs(slw_prestmt mstmt(databaseA, sql.data(), sql.size());)
	INIT_TRYs(slw_prestmt sstmt(databaseA, LITERAL_COMMA_SIZE("select * from sp_text_notes"));)
	for(;;){
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
				ifstre_.read(sybuf.get(),SYNOPSISLEN);
				if(!ifstre_ && !ifstre_.eof()){LOG_ERRs throw 0;}
				auto cogcount=ifstre_.gcount();
				ib=sqlite3_column_bytes(sstmt.s.pstmt,2);
				if(ib>=len){
					UNSAFE_MEMIMEMu(sefn,filenm,ib,str,len,{},{goto write_note_info;})
				}
				if(ifstre_.gcount()<len)goto revert_synopsis;
				UNSAFE_MEMIMEMu(sesyno,sybuf.get(),ifstre_.gcount(),str,len,{},{goto write_note_info;})
				if(ifstre_.eof())goto revert_synopsis;
				memcpy(chbuf.get(),sybuf.get()+SYNOPSISLEN-(len-1),len-1);
				for(;;){
					if(!ifstre_.read(chbuf.get()+len-1,CHBUFSI-(len-1))){
						if(!ifstre_.eof()){LOG_ERRs throw 0;}
						if(!ifstre_.gcount())goto revert_synopsis;//if(len-1+ifstre_.gcount()<len)
					}
					UNSAFE_MEMIMEMu(se,chbuf.get(),len-1+ifstre_.gcount(),str,len,{},{goto write_note_info;})
					if(ifstre_.eof())goto revert_synopsis;
					memcpy(chbuf.get(),chbuf.get()+CHBUFSI-(len-1),len-1);
				}
				write_note_info:
				ifstre_.clear(); ifstre_.close();
				if(!ifstre_){LOG_ERRs throw 0;}
				ib=sqlite3_bind_int64(mstmt.s.pstmt,1,sqlite3_column_int64(sstmt.s.pstmt, 0));
				if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
				//ib=sqlite3_bind_blob(mstmt.s.pstmt, 2, sybuf.get(),cogcount, SQLITE_TRANSIENT_STATIC_BEF_FIN);
				//if(SQLITE_OK!=ib){LOG_Is(ib) throw 0;}
				ib=sqlite3_step(mstmt.s.pstmt);
				if(SQLITE_DONE!=ib){LOG_Is(ib) throw 0;}
				sqlite3_reset(mstmt.s.pstmt);
				//remove later if no use
				//vb.insert(vb.end(),dirnm,dirnm+sqlite3_column_bytes(sstmt.s.pstmt,1));
				//vb.push_back('/');
				//vb.insert(vb.end(),filenm,filenm+sqlite3_column_bytes(sstmt.s.pstmt,2)+1);
				continue;
				revert_synopsis:
				ifstre_.clear(); ifstre_.close();
				if(!ifstre_){LOG_ERRs throw 0;}
				continue;
			}
		case SQLITE_DONE:break;
		default:{LOG_Is(ib) throw 0;}
		}
		break;
	}
	CATCH_SET_SUE_THROWs(;)
	CATCH_SET_SUE_THROWs(;)
}
