# Gunz2013
Gunz The Duel MatchServer Emulator(2013)

1. Summary :
This is my rewritten MatchServer. Aiming to compatibility with MAIET's 2011/2012 client.

2. Client versions to be supported :
Currently I aiming with 2011 client compatibility. After 2011 features finished, I'll switch to the 2012 client.

3. Progresses/ToDos :
This project has been finished with following some notes :
1. Clan War is not implemented.
2. Battle Reward is not implemented.
3. Some cash features (Gamble Cash Item, Buy Spendable Item, Buy Set Item) are not implemented.
4. Some hard-codes.
5. wxWidgets dependencies.
6. You can't use as multiple-servers (e.g. Separating Clan Server and Quest Server.)

4. Third party libraries :
This software uses wxWidgets, SFMT, TinyThread++, TinyXML-2 and OTL.
+ Some codes from MAIET's 2011 source.

Basically I don't need credits for my code part. But please follow the license of the third party.

5. DBMS used :
This emulator only supports PostgreSQL. which meaning is not compatible with your current GunzDB.
Therefore, you need PostgreSQL and it's ODBC driver (32bit).

6. About internal SQL queries :
Because of my lacking SQL knowledge, the internals became bad.

<b> Credits: aV3PQmCJjM9L </b>
