/**
 *	@Project name: SAMP-l0nnyLanguage
 *	@Author: l0nger, <l0nger.programmer@gmail.com>
 *	@Date created: 01-09-2013 20:50
 *	@Date updated: 30-01-2014
 *	@Version: 0.5.30124
 *
 *	@Link package: http://github.com/l0nger/SAMP-L0nnyLanguage
 *	@Licence: GPLv2
 *
 * (c) 2013-2014, <l0nger.programmer@gmail.com>
 **/

/*
	Functions list:
	
	// Language
	native Language::load(Languages:langid, filename[]);
	native Language::getText(Languages:langid, key[]);
	native Language::unloadAll();
	
	// Per-player
	native SetPlayerLanguage(playerid, Languages:langid);
	native GetPlayerLanguage(playerid);
*/

#if !defined _included_l0nnylang
	#define _included_l0nnylang
#else
	#endinput
#endif

#include <a_samp>
#tryinclude <a_sampdb>
#include <sscanf>

#define Language:: Language_

enum Languages: (+=1)
{
	LANGUAGE_NONE=0,
	LANGUAGE_PL=1,
	LANGUAGE_ENG,
	MAX_LANGUAGES,
};

enum e_LangData 
{
	DB:eld_dbHandler,
	Languages:eld_langid
};

static stock
	Language::dbVar[_:MAX_LANGUAGES][e_LangData],
	Languages:PlayerLangValue[MAX_PLAYERS];

// ---------
// Functions
// ---------
stock Language::unloadAll()
{
	for(new i=_:LANGUAGE_NONE; i < _:MAX_LANGUAGES-1; i++)
	{
		if(Language::dbVar[i][eld_dbHandler]) 
		{
			db_close(Language::dbVar[i][eld_dbHandler]);
		}
	}
}

stock Language::load(Languages:langid, filename[])
{
	if(!filename[0] || filename[0] && !filename[1]) 
	{
		print("[L0nnyLanguages]: Nie podano pelnej nazwy pliku docelowego");
		return false;
	}
	if(!fexist(filename))
	{
		printf("[L0nnyLanguages]: Podany przez Ciebie plik (%s), nie istnieje w katalogu scriptfiles/", filename);
		return false;
	}
	
	new File:fHandler=fopen(filename, io_read), fName[32];
	if(!fHandler)
	{
		printf("[L0nnyLanguages]: Nie mozna otworzyc pliku %s", filename);
		return false;
	}
	string_copy(fName, filename, strlen(filename)), strdel(fName, strfind(fName, "."), strlen(fName)), format(fName, sizeof(fName), "lang-%s.db", fName);
	if(fexist(fName)) fremove(fName);
	
	new dbID=Language::getArrayFreeID();
	Language::dbVar[dbID][eld_langid]=langid;
	Language::dbVar[dbID][eld_dbHandler]=db_open(fName);
	db_query(Language::dbVar[dbID][eld_dbHandler], "PRAGMA foreign_keys=ON; PRAGMA synchronous=OFF; PRAGMA journal_mode=MEMORY; PRAGMA default_cache_size=10000; PRAGMA locking_mode=EXCLUSIVE;");
	db_query(Language::dbVar[dbID][eld_dbHandler], "CREATE TEMP TABLE tempLang (`key` PRIMARY KEY, `text`);");
	new textCount=0, tmpBuf[128+32+4], keyBuf[32], textBuf[128];
	printf("[L0nnyLanguages]: Trwa przetwarzanie danych prosze czekac...");
	while(fread(fHandler, tmpBuf))
	{
		/*  PARSOWANIE DANYCH */
		if(!tmpBuf[0] || tmpBuf[0] && !tmpBuf[1]) continue;
		if(tmpBuf[0]==';') continue; // comment char
		if(!sscanf(tmpBuf, "p<=>s[32]s[128]", keyBuf, textBuf))
		{
			tmpBuf[0]=EOS;
			utility_deleteSpaces(keyBuf);
			utility_deleteSpaces(textBuf);
			format(tmpBuf, sizeof(tmpBuf), "INSERT INTO tempLang (`key`, `text`) VALUES ('%s', '%s');", Language::safeEscape(keyBuf), Language::safeEscape(textBuf)); 
			db_query(Language::dbVar[dbID][eld_dbHandler], tmpBuf);
			textCount++;
		}
	}
	printf("[L0nnyLanguages]: Zaladowano %d rekordow z pliku %s", textCount, filename);
	return true;
}

stock Language::getText(Languages:langid, key[])
{
	new id=Language::findArrayID(langid);
	new tmpBuf[128+1], DBResult:rSQL;
	if(!Language::dbVar[_:id][eld_dbHandler]) return (tmpBuf=""), tmpBuf;
	if(!key[0] || key[0] && !key[1]) return (tmpBuf=""), tmpBuf;
	
	format(tmpBuf, sizeof(tmpBuf), "SELECT `text` FROM tempLang WHERE `key`='%s' LIMIT 1;", Language::safeEscape(key));
	rSQL=db_query(Language::dbVar[_:id][eld_dbHandler], tmpBuf);
	if(!db_num_rows(rSQL))
	{
		db_free_result(rSQL);
		return (tmpBuf=""), tmpBuf;
	} else {
		db_get_field(rSQL, 0, tmpBuf, sizeof(tmpBuf));
		db_free_result(rSQL);
		return tmpBuf;
	}
}

/*
	> SetPlayerLanguage
	
	@desc:
		Ustawienie jezyka dla gracza
	@params:
		(int) playerid == id gracza,
		(int) langid == id jezyka, zmienna z enumatora Language:
	@returns
		(int) 0/false == jezeli gracza nie ma na serwerze, jezeli funkcja wykona sie prawidlowo - zwróci id jezyka
*/
stock SetPlayerLanguage(playerid, Languages:langid)
{
	if(!IsPlayerConnected(playerid)) return false;
	PlayerLangValue[playerid]=langid;
	return _:langid;
}

/*
	> GetPlayerLanguage
	
	@desc:
		Pobieranie id jezyka danego gracza
	@params:
		(int) playerid == id gracza
	@returns
		(int) 0/false == jezeli gracza nie ma na serwerze, id jezyku == jezeli gracz jest na serwerze
*/
stock GetPlayerLanguage(playerid)
{
	if(!IsPlayerConnected(playerid)) return false;
	return PlayerLangValue[playerid];
}

// Languages scripting functions
static stock Language::getArrayFreeID()
{
	for(new i=_:LANGUAGE_NONE; i < _:MAX_LANGUAGES-1; i++)
	{
		if(Language::dbVar[i][eld_dbHandler] == DB:0) 
		{
			return i;
		}
	}
	return false;
}

stock Language::safeEscape(text[])
{
    new ret[160], ch, i, j;
    while((ch = text[i++]) && j < sizeof(ret))
    {
        if(ch == '\'')
        {
            if(j < sizeof (ret) - 2)
            {
                ret[j++] = '\'';
                ret[j++] = '\'';
            }
        }
        else if(j < sizeof (ret))
        {
            ret[j++] = ch;
        }
        else
        {
            j++;
        }
    }
    ret[sizeof(ret) - 1] = '\0';
    return ret;
}

static stock Language::findArrayID(Languages:langid)
{
	for(new i=_:LANGUAGE_NONE; i < _:MAX_LANGUAGES-1; i++)
	{
		if(Language::dbVar[i][eld_langid] == langid) 
		{
			return i;
		}
	}
	return _:LANGUAGE_NONE;
}

// Utility functions
stock string_copy(dest[], const source[], length = sizeof(source))
{
	strins((dest[0] = EOS, dest), source, 0, length);
}

stock utility_deleteSpaces(string[])
{
	for(new i=strlen(string)-1; i>=0; i--)
	{
		if(string[i]==' ') string[i]='\0';
		else if(string[i]=='\n') string[i]='\0';
		else if(string[i]=='\r') string[i]='\0';
	}
}