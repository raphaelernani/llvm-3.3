/* translation of file "error.k" */
/* generated by:
 *  @(#)$Author: criswell $
 */
#define KC_FUNCTIONS_error_

#include <stdlib.h>
#include "k.h"
#include "error.h"
namespace kc { }
using namespace kc;
/* included stuff */
//
// The Termprocessor Kimwitu++
//
// Copyright (C) 1991 University of Twente, Dept TIOS.
// Copyright (C) 1998-2003 Humboldt-University of Berlin, Institute of Informatics
// All rights reserved.
//
// Kimwitu++ is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Kimwitu++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Kimwitu++; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

static char error_kAccesSid[] = "@(#)$Id: error.cc 12720 2004-04-06 20:25:22Z criswell $";

bool gp_no_fatal_problems;

#define QUOTEDBACKSLASH '\\'

#ifndef KC_MAX_STRING_LEN
# define KC_MAX_STRING_LEN 200
#endif /* !KC_MAX_STRING_LEN */

viewnameoption ug_viewnameopt;

int kc_filePrinter::indent_level = 4;
kc_filePrinter::kc_filePrinter(FILE* f):file(f)
{
    lineno=0;
    no_of_printed_string_chars=0;
    doit=false;
    lastChar='\n';
    indent=0;
    bs_cnt=0;
    inString=false;
    inChar=false;
    inComment=false;
    inCppComment=false;
    spacePending=false;
    beginOfLine = false;
    keyword=0;
    indentKeyword=false;
    inPreProStmt=false;
}

kc_filePrinter v_stdout_printer(stdout);
kc_filePrinter v_hfile_printer;
kc_filePrinter v_ccfile_printer;
printer_functor_class v_null_printer;

bool kc_filePrinter::check_keyword(const char* s)
{
    bool res=false;
    if(inPreProStmt) { // just to ignore #if and #else
	if(!isspace(*s))
	    inPreProStmt=false;
    }
    else if(!keyword) {
	if(*s=='#')
	    inPreProStmt=true;
	else if(*s=='i' || *s=='e' || *s=='d' || *s=='w') // 'if', 'else', 'do', 'while'
	    keyword=s; // 'for' not supported yet
    }
    else if(!isalnum(*s) && *s!='_') { // end of identifier
	ptrdiff_t length=s-keyword;
	if(
		(length==2 && strncmp(keyword,"if",length)==0) ||
		(length==4 && strncmp(keyword,"else",length)==0) ||
		(length==2 && strncmp(keyword,"do",length)==0) ||
		(length==5 && strncmp(keyword,"while",length)==0))
	    res=true; // keyword found
	keyword=0;
    }
    return res;
}

void kc_filePrinter::operator()(const char *s, uview v)
{
    char c;

    assertCond(file != 0);

    switch(v) {
	case view_no_of_printed_string_chars_reset_enum:
	no_of_printed_string_chars = 0;
	break;
	case view_printer_outputfileline_enum:
	fprintf( file, "\n" ); lineno = lineno +1;
	if(g_options.linedirec)
	    fprintf( file, "%s %d \"%s%s\"\n", pg_line, lineno+1, g_options.dir_line.c_str(),filename.c_str() );
	lineno = lineno +1;
	lastChar='\n';
	break;
	default:
	while((c=*s++)) {
	    switch( c ) {
		case '\0': return;
		case '\n':
		/* if (*s == QUOTEDBACKSLASH) s++; */
		lineno = lineno +1;
		beginOfLine = true;
		/* FALLTHROUGH */
		default:
		if (v == view_gen_unpstr_c) {
		    if (no_of_printed_string_chars >= KC_MAX_STRING_LEN) {
			if (doit) {
			    fprintf( file, "\"), " );
			    ug_viewnameopt->unparse( *this, view_gen_unparsedefs_other_c );
			    fprintf( file, " );\n		  kc_printer(kc_t(\"" );
			    lineno = lineno +1;
			    no_of_printed_string_chars = 0;
			    doit = false;
			} else {
			    switch( c ) {
				case '\\':
				case '\n':
				break;
				default:
				doit = true;
			    }
			}
		    }
		    no_of_printed_string_chars = no_of_printed_string_chars +1;
		} else if (v == view_filename) {
		    /* duplicate (= escape) backslashes in file names.
		     * we do this to help those that work on windows etc.
		     */
		    if (c == QUOTEDBACKSLASH) { /* we have to quote it! */
			putc( c, file );
		    }
		}
		if(inString) {
		    if(c=='"' && bs_cnt%2==0)
			inString=false;
		    putc( c, file );
		    lastChar=c;
		}
		else if(inChar) {
		    if(c=='\'' && bs_cnt%2==0)
			inChar=false;
		    putc( c, file );
		    lastChar=c;
		}
		else if(inComment) {
		    if(c=='/' && lastChar=='*')
			inComment=false; /* C comments */
		    switch(c) {
			case '\v': case '\r': case '\b': break;
			default:
			putc( c, file );
			lastChar=c;
		    }
		}
		else if(inCppComment) {
		    if(c=='\n')
			inCppComment=false; /* C++ comments */
		    switch(c) {
			case '\v': case '\r': case '\b': break;
			default:
			putc( c, file );
			lastChar=c;
		    }
		}
		else {
		    int indent_offset=0;
		    if(!indentKeyword) {
			indentKeyword=check_keyword(s-1);
			if(indentKeyword)
			    ++indent;
		    }
		    switch(c) {
			case ';':
			if(indentKeyword) {
			    --indent;
			    indentKeyword=false;
			}
			goto default_case;
			case '{':
			if(indentKeyword) {
			    --indent;
			    indentKeyword=false;
			}
			// no break
			case '(':
			indent_offset=1;
			goto default_case;
			case '\v':
			++indent;
			break;
			case '}':
			case ')':
			if(indent) --indent;
			goto default_case;
			case '\r':
			if(indent) --indent;
			break;
			case '\b':
			lastChar=c;
			break;
			case ' ':
			case '\t':
			if(lastChar=='\b' || !beginOfLine)
			    goto default_case;
			if(isspace(lastChar))
			    break;
			if(isalnum(lastChar) || lastChar=='_' || lastChar=='"' || lastChar=='\'' || lastChar==')' || lastChar=='}') {
			    if(isalnum(*(s+1))|| *(s+1)=='_'|| *(s+1)=='"' || *(s+1)=='\'') {
				c=' ';
				goto default_case;
			    }
			    spacePending=true;
			}
			break;
			default:
			default_case:
			if(lastChar=='\n' && c!='\n' && c!='#') {
			    for(int i=indent*indent_level;i>0;)
			    if(i>=8) {
				putc('\t',file);
				i-=8;
			    } else {
				for(int k=0;k<i;k++)
				putc(' ',file);
				i=0;
			    }
			}
			if(!isspace(c))
			    beginOfLine = false;
			if(c=='"' && bs_cnt%2==0)
			    inString=true;
			else if(c=='\'' && bs_cnt%2==0)
			    inChar=true;
			else if(c=='/' && lastChar=='/')
			    inCppComment=true; /* C++ comments */
			else if(c=='*' && lastChar=='/')
			    inComment=true; /* C comments */
			if(spacePending) {
			    if(isalnum(c)|| c=='_' || c=='"' || c=='\'')
				putc( ' ', file );
			    spacePending=false;
			}
			putc( c, file );
			lastChar=c;
			indent+=indent_offset;
		    }
		}
		if(c=='\\')
		    ++bs_cnt;
		else
		    bs_cnt=0;
	    }
	}
	keyword=0; // no keyword check between different strings
    }
}

void kc_filePrinter::init(const char *name, const char *mode, const string &_filename)
{
    file=fopen(name, mode);
    if (file==0) v_report( Fatal( NoFileLine(), Problem4S( "cannot create temporary ", _filename.c_str(), " file:", name )));
    lineno = 1;
    filename = _filename;
    lastChar='\n';
    indent=0;
    inString=false;
    inComment=false;
    inCppComment=false;
    spacePending=false;
    beginOfLine=false;
}



/* end included stuff */


namespace kc {

#ifndef KC_TRACE_PROVIDED
#define KC_TRACE_PROVIDED(COND,FILE,LINE,NODE) COND
#endif

static  void v_stderr_printer (const char *s, uview v);
problem Problem1S(const char *s1)
{
    return Problem1( mkcasestring( s1 ));

}

problem Problem1S1we(const char *s1, withexpression we)
{
    return Problem1we( mkcasestring( s1 ), we );

}

problem Problem1S1ID(const char *s1, ID id)
{
    return Problem1ID( mkcasestring( s1 ), id );

}

problem Problem1S1tID(const char *s1, ID id)
{
    return Problem1tID( mkcasestring( s1 ), id );

}

problem Problem1S1ID1S1ID(const char *s1, ID id1, const char *s2, ID id2)
{
    return Problem1ID1ID( mkcasestring( s1 ), id1, mkcasestring( s2 ), id2 );

}

problem Problem1S1t1S1ID(const char *s1, IDtype id1, const char *s2, ID id2)
{
    return Problem1t1ID( mkcasestring( s1 ), id1, mkcasestring( s2 ), id2 );

}

problem Problem1S1INT(const char *s1, INT i1)
{
    return Problem1INT( mkcasestring( s1 ), i1 );

}

problem Problem1S1int1S(const char *s1, int i1, const char *s2)
{
    return Problem1int1( mkcasestring( s1 ), mkinteger(i1), mkcasestring( s2 ) );

}

problem Problem1S1INT1S1ID(const char *s1, INT i1, const char *s2, ID id2)
{
    return Problem1INT1ID( mkcasestring( s1 ), i1, mkcasestring( s2 ), id2 );

}

problem Problem1S1ID1S1ID1S1ID(const char *s1, ID id1, const char *s2, ID id2, const char *s3, ID id3)
{
    return Problem1ID1ID1ID( mkcasestring( s1 ), id1, mkcasestring( s2 ), id2, mkcasestring( s3 ), id3 );

}

problem Problem1S1INT1S1ID1S1ID(const char *s1, INT i1, const char *s2, ID id2, const char *s3, ID id3)
{
    return Problem1INT1ID1ID( mkcasestring( s1 ), i1, mkcasestring( s2 ), id2, mkcasestring( s3 ), id3 );

}

problem Problem1S1storageoption1S1ID(const char *s1, storageoption so, const char *s2, ID id)
{
    return Problem1storageoption1ID( mkcasestring( s1 ), so, mkcasestring( s2 ), id );

}

problem Problem2S(const char *s1, const char *s2)
{
    return Problem2( mkcasestring( s1 ), mkcasestring( s2 ));

}

problem ProblemSC(const char *s1, casestring s2)
{
    return Problem2( mkcasestring( s1 ), s2 );

}

problem Problem3S(const char *s1, const char *s2, const char *s3)
{
    return Problem3( mkcasestring( s1 ), mkcasestring( s2 ), mkcasestring( s3 ));

}

problem Problem4S(const char *s1, const char *s2, const char *s3, const char *s4)
{
    return Problem4( mkcasestring( s1 ), mkcasestring( s2 ), mkcasestring( s3 ), mkcasestring( s4 ) );

}

problem Problem3S1int1S(const char *s1, const char *s2, const char *s3, int i1, const char *s4)
{
    return Problem3int1( mkcasestring( s1 ), mkcasestring( s2 ), mkcasestring( s3 ), mkinteger(i1), mkcasestring( s4 ) );

}

void v_report(error e)
{
    if(g_options.msg_format.length()) {
	view_error_format_class v(g_options.msg_format);
	e->unparse( v_stderr_printer, v );
    }
    else
    e->unparse( v_stderr_printer, view_error );

}

static  void v_stderr_printer(const char *s, uview v)
{
    fflush( stdout );
    fprintf( stderr, "%s", s );
    fflush( stderr );

}


} // namespace kc