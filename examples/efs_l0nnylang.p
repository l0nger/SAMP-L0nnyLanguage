#include <a_samp>
#include <l0nnyLanguage>

public OnFilterScriptInit()
{
	/*new File:f=fopen("polish.txt", io_append);
	new buf[100];
	for(new i; i<500; i++)
	{
		format(buf, sizeof(buf), "txt_player%d = player%d\r\n", i, i);
		fwrite(f, buf);
	}
	fclose(f);*/
	Language::load(LANGUAGE_PL, "polish.txt");
	
	new buf[40];
	new tick=GetTickCount();
	for(new i; i<1000; i++)
	{
	//	format(buf, sizeof(buf), "txt_player%d", random(499));
		Language::getText(LANGUAGE_PL, "txt_player499");
		// 
	}
	printf("wykonano w: %d ms", GetTickCount()-tick);
	printf("%s", Language::getText(LANGUAGE_PL, "txt_player499"));
	return true;
}

public OnFilterScriptExit()
{
	Language::unloadAll();
	return true;
}