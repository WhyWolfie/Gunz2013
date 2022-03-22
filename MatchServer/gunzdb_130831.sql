--
-- PostgreSQL database dump
--

-- Dumped from database version 9.2.4
-- Dumped by pg_dump version 9.2.4
-- Started on 2013-08-31 06:28:59

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- TOC entry 199 (class 3079 OID 11727)
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- TOC entry 2190 (class 0 OID 0)
-- Dependencies: 199
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

--
-- TOC entry 295 (class 1255 OID 17132)
-- Name: acceptgift(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION acceptgift(id_ integer, cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE gift SET cid = NULL WHERE id = id_ AND cid = cid_;$$;


--
-- TOC entry 213 (class 1255 OID 16579)
-- Name: addbannedip(inet, timestamp with time zone); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION addbannedip(ip inet, date timestamp with time zone DEFAULT NULL::timestamp with time zone) RETURNS void
    LANGUAGE sql
    AS $$INSERT INTO bannediplist (addr, period) VALUES (ip, date);$$;


--
-- TOC entry 284 (class 1255 OID 17106)
-- Name: addblitzshopitem(integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION addblitzshopitem(cid_ integer, itemid_ integer, price_ integer, baseprice_ integer, quantity_ integer, renthourperiod_ integer) RETURNS void
    LANGUAGE sql
    AS $$INSERT INTO blitzshop (cid, itemid, price, baseprice, quantity, renthourperiod) 
VALUES (cid_, itemid_, price_, baseprice_, quantity_, renthourperiod_);$$;


--
-- TOC entry 214 (class 1255 OID 16580)
-- Name: addfriend(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION addfriend(cid_ integer, targetcid_ integer) RETURNS integer
    LANGUAGE sql
    AS $$INSERT INTO friend (cid, friendcid) VALUES (cid_, targetcid_) RETURNING id;$$;


--
-- TOC entry 215 (class 1255 OID 16581)
-- Name: banplayer(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION banplayer(aid_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

IF NOT EXISTS(SELECT 1 FROM account WHERE aid = aid_ LIMIT 1) 
THEN 
   RETURN; 
END IF;

SELECT changeaccountugradeid(aid_, 253 /* banned. */);
SELECT addbannedip(CAST((SELECT lastip FROM account WHERE aid = aid_ LIMIT 1) AS inet));

END;$$;


--
-- TOC entry 216 (class 1255 OID 16582)
-- Name: changeaccountugradeid(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION changeaccountugradeid(aid_ integer, ugradeid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE account SET ugradeid = ugradeid_ WHERE aid = aid_;$$;


--
-- TOC entry 290 (class 1255 OID 17101)
-- Name: checkexpiredaccountitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION checkexpiredaccountitem(aid_ integer, OUT aiid_ integer, OUT itemid_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$UPDATE accountitem SET aid = NULL WHERE 
aid = aid_ AND expiredate IS NOT NULL AND 
expiredate <= CURRENT_TIMESTAMP 
RETURNING aiid, itemid;$$;


--
-- TOC entry 217 (class 1255 OID 16583)
-- Name: checkexpireditem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION checkexpireditem(cid_ integer, OUT ciid_ integer, OUT itemid_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$UPDATE characteritem SET cid = NULL WHERE 
cid = cid_ AND expiredate IS NOT NULL AND 
expiredate <= CURRENT_TIMESTAMP 
RETURNING ciid, itemid;$$;


--
-- TOC entry 297 (class 1255 OID 17134)
-- Name: checkgift(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION checkgift(cid_ integer, OUT id_ integer, OUT sender_ character varying, OUT message_ character varying, OUT itemid1_ integer, OUT itemid2_ integer, OUT itemid3_ integer, OUT itemid4_ integer, OUT itemid5_ integer, OUT renthourperiod_ integer, OUT giftdate_ character varying) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT 
    id, 
    sender, 
    message, 
    itemid[1], 
    itemid[2], 
    itemid[3], 
    itemid[4], 
    itemid[5], 
    renthourperiod, 
    to_char(giftdate, 'YYYY MM DD HH24') 
FROM
    gift
WHERE
    cid = cid_;$$;


--
-- TOC entry 282 (class 1255 OID 17104)
-- Name: clearblitzshop(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION clearblitzshop() RETURNS void
    LANGUAGE sql
    AS $$DELETE FROM blitzshop;$$;


--
-- TOC entry 283 (class 1255 OID 17105)
-- Name: clearblitzshop(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION clearblitzshop(cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$DELETE FROM blitzshop WHERE cid = cid_;$$;


--
-- TOC entry 218 (class 1255 OID 16584)
-- Name: clearfriend(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION clearfriend(targetcid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE friend SET deleteflag = TRUE WHERE friendcid = targetcid_;$$;


--
-- TOC entry 219 (class 1255 OID 16585)
-- Name: closeclan(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION closeclan(clid_ integer) RETURNS void
    LANGUAGE sql
    AS $$DELETE FROM clanmember WHERE clid = clid_;
UPDATE clan SET name = NULL, deleteflag = TRUE, deletename = name WHERE clid = clid_;$$;


--
-- TOC entry 220 (class 1255 OID 16586)
-- Name: createclan(integer, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION createclan(cid_ integer, clanname_ character varying) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
    clid_ integer DEFAULT 0;

BEGIN
    INSERT INTO clan (name, mastercid) VALUES (
       clanname_, 
       cid_
    ) RETURNING clid INTO clid_;

    INSERT INTO clanmember (clid, cid, grade) VALUES (
       clid_, cid_, 1 /* 1 : clan master grade. */
    );

    RETURN clid_;
END;$$;


--
-- TOC entry 221 (class 1255 OID 16587)
-- Name: decreasegambleitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION decreasegambleitem(cgiid_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE
   res_cnt integer DEFAULT 0;

BEGIN

UPDATE chargambleitem SET quantity = quantity - 1 WHERE cgiid = cgiid_ 
RETURNING quantity INTO res_cnt;

IF res_cnt <= 0 THEN
   UPDATE chargambleitem SET cid = NULL WHERE cgiid = cgiid_;
END IF;

END;$$;


--
-- TOC entry 272 (class 1255 OID 16588)
-- Name: deletecharacter(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION deletecharacter(id integer) RETURNS void
    LANGUAGE sql
    AS $$-- remove this char from friend list.
SELECT clearfriend(id);

DELETE FROM survival WHERE cid = id; -- survival info.
DELETE FROM dueltournament WHERE cid = id;  -- dt info.
DELETE FROM blitzkrieg WHERE cid = id; -- blitz info.

-- char itself.
UPDATE character SET name = NULL, deleteflag = TRUE, deletename = name, deletedate = CURRENT_TIMESTAMP WHERE cid = id;$$;


--
-- TOC entry 280 (class 1255 OID 17083)
-- Name: deletegambleitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION deletegambleitem(cgiid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE chargambleitem SET cid = NULL WHERE cgiid = cgiid_;$$;


--
-- TOC entry 227 (class 1255 OID 16589)
-- Name: deletegambleitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION deletegambleitem(cid_ integer, cgiid_ integer, newbp_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT deletegambleitem(cgiid_);$$;


--
-- TOC entry 276 (class 1255 OID 17079)
-- Name: deleteitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION deleteitem(ciid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE characteritem SET cid = NULL WHERE ciid = ciid_;$$;


--
-- TOC entry 226 (class 1255 OID 16590)
-- Name: deleteitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION deleteitem(cid_ integer, ciid_ integer, newbp_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT deleteitem(ciid_);$$;


--
-- TOC entry 229 (class 1255 OID 16591)
-- Name: equipitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION equipitem(cid_ integer, ciid_ integer, slot_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

CASE slot_ 
    WHEN 0 THEN
        UPDATE character SET head_slot = ciid_ WHERE cid = cid_;
    WHEN 1 THEN
        UPDATE character SET chest_slot = ciid_ WHERE cid = cid_;
    WHEN 2 THEN
        UPDATE character SET hands_slot = ciid_ WHERE cid = cid_;
    WHEN 3 THEN
        UPDATE character SET legs_slot = ciid_ WHERE cid = cid_;
    WHEN 4 THEN
        UPDATE character SET feet_slot = ciid_ WHERE cid = cid_;
    WHEN 5 THEN
        UPDATE character SET fingerl_slot = ciid_ WHERE cid = cid_;
    WHEN 6 THEN
        UPDATE character SET fingerr_slot = ciid_ WHERE cid = cid_;
    WHEN 7 THEN
        UPDATE character SET melee_slot = ciid_ WHERE cid = cid_;
    WHEN 8 THEN
        UPDATE character SET primary_slot = ciid_ WHERE cid = cid_;
    WHEN 9 THEN
        UPDATE character SET secondary_slot = ciid_ WHERE cid = cid_;
    WHEN 10 THEN
        UPDATE character SET custom1_slot = ciid_ WHERE cid = cid_;
    WHEN 11 THEN
        UPDATE character SET custom2_slot = ciid_ WHERE cid = cid_;
    WHEN 12 THEN
        UPDATE character SET avatar_slot = ciid_ WHERE cid = cid_;
    WHEN 13 THEN
        UPDATE character SET community1_slot = ciid_ WHERE cid = cid_;
    WHEN 14 THEN
        UPDATE character SET community2_slot = ciid_ WHERE cid = cid_;
    WHEN 15 THEN
        UPDATE character SET longbuff1_slot = ciid_ WHERE cid = cid_;
    WHEN 16 THEN
        UPDATE character SET longbuff2_slot = ciid_ WHERE cid = cid_;
ELSE END CASE;

END;$$;


--
-- TOC entry 230 (class 1255 OID 16592)
-- Name: extractsecperiod(timestamp with time zone); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION extractsecperiod(time_ timestamp with time zone) RETURNS integer
    LANGUAGE sql
    AS $$SELECT CAST(EXTRACT(EPOCH FROM (time_ - CURRENT_TIMESTAMP)) AS integer);$$;


--
-- TOC entry 231 (class 1255 OID 16593)
-- Name: fetchdttopranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION fetchdttopranking(OUT charname_ character varying, OUT tp_ integer, OUT win_ integer, OUT lose_ integer, OUT ranking_ integer, OUT rankingdiff_ integer, OUT finalwin_ integer, OUT class_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT 
  "character".name, 
  dueltournament.tp, 
  dueltournament.win, 
  dueltournament.lose, 
  dueltournament.ranking, 
  dueltournament.rankingdiff, 
  dueltournament.finalwin, 
  dueltournament.class
FROM 
  dueltournament, 
  "character"
WHERE 
  dueltournament.cid = "character".cid

ORDER BY ranking ASC LIMIT 5;$$;


--
-- TOC entry 273 (class 1255 OID 17066)
-- Name: fetchsurvivalranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION fetchsurvivalranking(OUT charname_ character varying, OUT point_ integer, OUT ranking_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT "character".name, survival.point, survival.ranking 
FROM survival, "character" 
WHERE survival.cid = "character".cid 
ORDER BY survival.ranking ASC LIMIT 10;$$;


--
-- TOC entry 289 (class 1255 OID 17100)
-- Name: getaccitemlist(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getaccitemlist(aid_ integer, OUT aiid_ integer, OUT itemid_ integer, OUT period_ integer, OUT count_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT aiid, itemid, extractsecperiod(expiredate), quantity FROM accountitem WHERE aid = aid_;$$;


SET default_tablespace = '';

SET default_with_oids = false;

--
-- TOC entry 168 (class 1259 OID 16594)
-- Name: account; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE account (
    aid integer NOT NULL,
    userid character varying(128) NOT NULL,
    password character varying(128),
    ugradeid integer DEFAULT 0 NOT NULL,
    pgradeid integer DEFAULT 0 NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    lastconndate timestamp with time zone,
    lastip character varying(64),
    cash integer DEFAULT 0 NOT NULL
);


--
-- TOC entry 232 (class 1255 OID 16600)
-- Name: getaccountinfo(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getaccountinfo(id character varying) RETURNS SETOF account
    LANGUAGE sql
    AS $$SELECT * FROM account WHERE userid ILIKE id LIMIT 1;$$;


--
-- TOC entry 233 (class 1255 OID 16601)
-- Name: getbasicclaninfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getbasicclaninfo(cid_in integer, OUT clid_out integer, OUT name_out character varying, OUT grade_out integer, OUT point_out integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT clan.clid, clan.name, clanmember.grade, clan.point FROM clan, clanmember WHERE 
clan.clid = clanmember.clid AND clanmember.cid = cid_in LIMIT 1;$$;


--
-- TOC entry 190 (class 1259 OID 16781)
-- Name: blitzkrieg; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE blitzkrieg (
    id integer NOT NULL,
    cid integer NOT NULL,
    win integer DEFAULT 0 NOT NULL,
    lose integer DEFAULT 0 NOT NULL,
    point integer DEFAULT 1000 NOT NULL,
    medal integer DEFAULT 0 NOT NULL
);


--
-- TOC entry 292 (class 1255 OID 17103)
-- Name: getblitzscore(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getblitzscore(cid_ integer) RETURNS SETOF blitzkrieg
    LANGUAGE sql
    AS $$SELECT * FROM blitzkrieg WHERE cid = cid_ LIMIT 1;$$;


--
-- TOC entry 192 (class 1259 OID 16793)
-- Name: blitzshop; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE blitzshop (
    id integer NOT NULL,
    cid integer NOT NULL,
    itemid integer NOT NULL,
    price integer NOT NULL,
    baseprice integer NOT NULL,
    quantity integer DEFAULT 1 NOT NULL,
    renthourperiod integer DEFAULT 0 NOT NULL
);


--
-- TOC entry 228 (class 1255 OID 17108)
-- Name: getblitzshopitem(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getblitzshopitem() RETURNS SETOF blitzshop
    LANGUAGE sql
    AS $$SELECT * FROM blitzshop;$$;


--
-- TOC entry 285 (class 1255 OID 17107)
-- Name: getblitzshopitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getblitzshopitem(cid_ integer) RETURNS SETOF blitzshop
    LANGUAGE sql
    AS $$SELECT * FROM blitzshop WHERE cid = cid_;$$;


--
-- TOC entry 234 (class 1255 OID 16602)
-- Name: getcharactercount(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharactercount(id integer) RETURNS integer
    LANGUAGE sql
    AS $$SELECT CAST(COUNT(*) AS integer) FROM character WHERE aid = id AND deleteflag = FALSE;$$;


--
-- TOC entry 169 (class 1259 OID 16603)
-- Name: character; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE "character" (
    cid integer NOT NULL,
    aid integer NOT NULL,
    name character varying(24),
    level integer DEFAULT 1 NOT NULL,
    sex integer NOT NULL,
    hair integer NOT NULL,
    face integer NOT NULL,
    exp integer DEFAULT 0 NOT NULL,
    bounty integer DEFAULT 0 NOT NULL,
    head_slot integer,
    chest_slot integer,
    hands_slot integer,
    legs_slot integer,
    feet_slot integer,
    fingerl_slot integer,
    fingerr_slot integer,
    melee_slot integer,
    primary_slot integer,
    secondary_slot integer,
    custom1_slot integer,
    custom2_slot integer,
    avatar_slot integer,
    community1_slot integer,
    community2_slot integer,
    longbuff1_slot integer,
    longbuff2_slot integer,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    killcount integer DEFAULT 0 NOT NULL,
    deathcount integer DEFAULT 0 NOT NULL,
    ranking integer,
    deleteflag boolean DEFAULT false NOT NULL,
    deletename character varying(24),
    deletedate timestamp with time zone,
    questiteminfo character varying(1024)
);


--
-- TOC entry 235 (class 1255 OID 16616)
-- Name: getcharacterinfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharacterinfo(id integer) RETURNS SETOF "character"
    LANGUAGE sql
    AS $$SELECT * FROM character WHERE cid = id LIMIT 1;$$;


--
-- TOC entry 236 (class 1255 OID 16617)
-- Name: getcharacterlist(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharacterlist(id integer, OUT charname character varying, OUT charnum integer, OUT charlevel integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT name, CAST(ROW_NUMBER() OVER(ORDER BY cid ASC) AS integer) - 1, level FROM character WHERE aid = id AND deleteflag = FALSE;$$;


--
-- TOC entry 237 (class 1255 OID 16618)
-- Name: getcharalliteminfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharalliteminfo(cid_ integer, OUT itemid_ integer, OUT quantity_ integer, OUT slotnum_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$BEGIN

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  0 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".head_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  1 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".chest_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  2 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".hands_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  3 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".legs_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  4 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".feet_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  5 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".fingerl_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  6 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".fingerr_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  7 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".melee_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  8 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".primary_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  9 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".secondary_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  10 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".custom1_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  11 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".custom2_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  12 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".avatar_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  13 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".community1_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  14 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".community2_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  15 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".longbuff1_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;

SELECT 
  characteritem.itemid, 
  characteritem.quantity, 
  16 
FROM 
  public."character", 
  public.characteritem 
WHERE 
  characteritem.ciid = "character".longbuff2_slot AND 
  "character".cid = cid_ 
  INTO itemid_, quantity_, slotnum_;
RETURN NEXT;


RETURN;

END;$$;


--
-- TOC entry 238 (class 1255 OID 16619)
-- Name: getcharequipmentslotciid(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharequipmentslotciid(cid_ integer, OUT ciid_ integer, OUT slotnum_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$BEGIN

SELECT head_slot, 0 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT chest_slot, 1 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT hands_slot, 2 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT legs_slot, 3 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT feet_slot, 4 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT fingerl_slot, 5 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT fingerr_slot, 6 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT melee_slot, 7 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT primary_slot, 8 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT secondary_slot, 9 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT custom1_slot, 10 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT custom2_slot, 11 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT avatar_slot, 12 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT community1_slot, 13 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT community2_slot, 14 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT longbuff1_slot, 15 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;
	
SELECT longbuff2_slot, 16 FROM character WHERE cid = cid_ INTO ciid_, slotnum_;
	RETURN NEXT;

RETURN;

END;$$;


--
-- TOC entry 170 (class 1259 OID 16620)
-- Name: chargambleitem; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE chargambleitem (
    cgiid integer NOT NULL,
    cid integer,
    itemid integer NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    quantity integer NOT NULL
);


--
-- TOC entry 239 (class 1255 OID 16624)
-- Name: getchargambleitem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getchargambleitem(cid_ integer) RETURNS SETOF chargambleitem
    LANGUAGE sql
    AS $$SELECT * FROM chargambleitem WHERE cid = cid_ ORDER BY itemid ASC;$$;


--
-- TOC entry 240 (class 1255 OID 16625)
-- Name: getcharindex(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharindex(id integer, OUT charid integer, OUT charnum integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT cid, CAST(ROW_NUMBER() OVER(ORDER BY cid ASC) AS integer) - 1 
FROM character WHERE aid = id AND deleteflag = FALSE;$$;


--
-- TOC entry 241 (class 1255 OID 16626)
-- Name: getcharitemlist(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcharitemlist(cid_ integer, OUT ciid_ integer, OUT itemid_ integer, OUT period_ integer, OUT count_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT ciid, itemid, extractsecperiod(expiredate), quantity FROM characteritem WHERE cid = cid_;$$;


--
-- TOC entry 171 (class 1259 OID 16627)
-- Name: clan; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE clan (
    clid integer NOT NULL,
    name character varying(24),
    level integer DEFAULT 1 NOT NULL,
    point integer DEFAULT 1000 NOT NULL,
    mastercid integer,
    win integer DEFAULT 0 NOT NULL,
    lose integer DEFAULT 0 NOT NULL,
    draw integer DEFAULT 0 NOT NULL,
    totalpoint integer DEFAULT 0 NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    ranking integer,
    emblemurl character varying(256),
    emblemchecksum integer DEFAULT 0 NOT NULL,
    deleteflag boolean DEFAULT false NOT NULL,
    deletename character varying(24)
);


--
-- TOC entry 200 (class 1255 OID 16639)
-- Name: getclaninfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getclaninfo(clid_ integer) RETURNS SETOF clan
    LANGUAGE sql
    AS $$SELECT * FROM clan WHERE clid = clid_;$$;


--
-- TOC entry 222 (class 1255 OID 16640)
-- Name: getcqrecord(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getcqrecord(aid_ integer, OUT scenarioid_ integer, OUT time_ integer) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT scenarioid, MIN(time) FROM challengequestrecord WHERE aid = aid_ GROUP BY scenarioid;$$;


--
-- TOC entry 172 (class 1259 OID 16641)
-- Name: dueltournament; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE dueltournament (
    dtrid integer NOT NULL,
    cid integer NOT NULL,
    tp integer DEFAULT 1000 NOT NULL,
    win integer DEFAULT 0 NOT NULL,
    lose integer DEFAULT 0 NOT NULL,
    finalwin integer DEFAULT 0 NOT NULL,
    ranking integer,
    tpprev integer DEFAULT 0 NOT NULL,
    winprev integer DEFAULT 0 NOT NULL,
    loseprev integer DEFAULT 0 NOT NULL,
    finalwinprev integer DEFAULT 0 NOT NULL,
    rankingprev integer,
    rankingdiff integer,
    class integer DEFAULT 10 NOT NULL
);


--
-- TOC entry 223 (class 1255 OID 16653)
-- Name: getdtcharinfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getdtcharinfo(id integer) RETURNS SETOF dueltournament
    LANGUAGE sql
    AS $$SELECT * FROM dueltournament WHERE cid = id LIMIT 1;$$;


--
-- TOC entry 224 (class 1255 OID 16654)
-- Name: getfriendlist(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getfriendlist(cid_ integer, OUT targetid_ integer, OUT targetcid_ integer, OUT targetname_ character varying) RETURNS SETOF record
    LANGUAGE sql
    AS $$SELECT friend.id, friend.friendcid, character.name FROM character, friend 
WHERE character.cid = friend.friendcid AND friend.cid = cid_ 
AND friend.deleteflag = FALSE ORDER BY friend.id ASC;$$;


--
-- TOC entry 173 (class 1259 OID 16655)
-- Name: serverstatus; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE serverstatus (
    id integer NOT NULL,
    currplayer integer,
    maxplayer integer,
    "time" timestamp with time zone,
    ip character varying(64),
    port integer,
    name character varying(64),
    opened boolean,
    type integer,
    agentip character varying(64)
);


--
-- TOC entry 225 (class 1255 OID 16658)
-- Name: getserverstatus(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getserverstatus() RETURNS SETOF serverstatus
    LANGUAGE sql
    AS $$SELECT * FROM serverstatus WHERE opened = TRUE ORDER BY id ASC;$$;


--
-- TOC entry 194 (class 1259 OID 17037)
-- Name: survival; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE survival (
    id integer NOT NULL,
    cid integer NOT NULL,
    point integer DEFAULT 0 NOT NULL,
    ranking integer
);


--
-- TOC entry 270 (class 1255 OID 17067)
-- Name: getsurvivalcharinfo(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION getsurvivalcharinfo(cid_ integer) RETURNS SETOF survival
    LANGUAGE sql
    AS $$SELECT * FROM survival WHERE cid = cid_ LIMIT 1;$$;


--
-- TOC entry 269 (class 1255 OID 16659)
-- Name: insertaccount(character varying, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertaccount(userid_ character varying, password_ character varying) RETURNS integer
    LANGUAGE plpgsql
    AS $$BEGIN

IF EXISTS(SELECT 1 FROM account WHERE userid ILIKE userid_ LIMIT 1) 
THEN
    RETURN 0;
END IF;

INSERT INTO account (userid, password) VALUES (userid_, password_);
RETURN 1;

END;$$;


--
-- TOC entry 242 (class 1255 OID 16660)
-- Name: insertcharacter(integer, character varying, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertcharacter(accid integer, charname character varying, charsex integer, charhair integer, charface integer, head_id integer, chest_id integer, hands_id integer, legs_id integer, feet_id integer, fingerl_id integer, fingerr_id integer, melee_id integer, primary_id integer, secondary_id integer, custom1_id integer, custom2_id integer, avatar_id integer, community1_id integer, community2_id integer, longbuff1_id integer, longbuff2_id integer) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE

-- character id.
charid integer DEFAULT 0;

-- character item id.
head_ciid integer DEFAULT 0;	-- head.
chest_ciid integer DEFAULT 0;	-- chest.
hands_ciid integer DEFAULT 0;	-- hands.
legs_ciid integer DEFAULT 0;	-- legs.
feet_ciid integer DEFAULT 0;	-- feet.
fingerl_ciid integer DEFAULT 0;	-- finger l.
fingerr_ciid integer DEFAULT 0;	-- finger r.
melee_ciid integer DEFAULT 0;	-- melee.
primary_ciid integer DEFAULT 0;	-- primary.
secondary_ciid integer DEFAULT 0;	-- secondary.
custom1_ciid integer DEFAULT 0;	-- custom 1.
custom2_ciid integer DEFAULT 0;	-- custom 2.
avatar_ciid integer DEFAULT 0;	-- avatar.
community1_ciid integer DEFAULT 0;	-- community 1.
community2_ciid integer DEFAULT 0;	-- community 2.
longbuff1_ciid integer DEFAULT 0;	-- longbuff 1.
longbuff2_ciid integer DEFAULT 0;	-- longbuff 2.


BEGIN

-- insert character.
INSERT INTO character (aid, name, sex, hair, face) 
VALUES (accid, charname, charsex, charhair, charface) 
RETURNING cid INTO charid;


-- insert character item.
IF head_id <> 0	THEN	-- head.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, head_id) RETURNING ciid INTO head_ciid;
END IF;

IF chest_id <> 0	THEN	-- chest.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, chest_id) RETURNING ciid INTO chest_ciid;
END IF;

IF hands_id <> 0	THEN	-- hands.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, hands_id) RETURNING ciid INTO hands_ciid;
END IF;

IF legs_id <> 0	THEN	-- legs.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, legs_id) RETURNING ciid INTO legs_ciid;
END IF;

IF feet_id <> 0	THEN	-- feet.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, feet_id) RETURNING ciid INTO feet_ciid;
END IF;

IF fingerl_id <> 0	THEN	-- finger l.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, fingerl_id) RETURNING ciid INTO fingerl_ciid;
END IF;

IF fingerr_id <> 0	THEN	-- finger r.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, fingerr_id) RETURNING ciid INTO fingerr_ciid;
END IF;

IF melee_id <> 0	THEN	-- melee.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, melee_id) RETURNING ciid INTO melee_ciid;
END IF;

IF primary_id <> 0	THEN	-- primary.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, primary_id) RETURNING ciid INTO primary_ciid;
END IF;

IF secondary_id <> 0	THEN	-- secondary.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, secondary_id) RETURNING ciid INTO secondary_ciid;
END IF;

IF custom1_id <> 0	THEN	-- custom 1.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, custom1_id) RETURNING ciid INTO custom1_ciid;
END IF;

IF custom2_id <> 0	THEN	-- custom 2.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, custom2_id) RETURNING ciid INTO custom2_ciid;
END IF;

IF avatar_id <> 0	THEN	-- avatar.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, avatar_id) RETURNING ciid INTO avatar_ciid;
END IF;

IF community1_id <> 0	THEN	-- community 1.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, community1_id) RETURNING ciid INTO community1_ciid;
END IF;

IF community2_id <> 0	THEN	-- community 2.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, community2_id) RETURNING ciid INTO community2_ciid;
END IF;

IF longbuff1_id <> 0	THEN	-- longbuff 1.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, longbuff1_id) RETURNING ciid INTO longbuff1_ciid;
END IF;

IF longbuff2_id <> 0	THEN	-- longbuff 2.
INSERT INTO characteritem (cid, itemid) 
VALUES (charid, longbuff2_id) RETURNING ciid INTO longbuff2_ciid;
END IF;


-- update character item slot.
IF head_ciid <> 0	THEN	-- head.
UPDATE character SET head_slot=head_ciid WHERE cid=charid;
END IF;

IF chest_ciid <> 0	THEN	-- chest.
UPDATE character SET chest_slot=chest_ciid WHERE cid=charid;
END IF;

IF hands_ciid <> 0	THEN	-- hands.
UPDATE character SET hands_slot=hands_ciid WHERE cid=charid;
END IF;

IF legs_ciid <> 0	THEN	-- legs.
UPDATE character SET legs_slot=legs_ciid WHERE cid=charid;
END IF;

IF feet_ciid <> 0	THEN	-- feet.
UPDATE character SET feet_slot=feet_ciid WHERE cid=charid;
END IF;

IF fingerl_ciid <> 0	THEN	-- finger l.
UPDATE character SET fingerl_slot=fingerl_ciid WHERE cid=charid;
END IF;

IF fingerr_ciid <> 0	THEN	-- finger r.
UPDATE character SET fingerr_slot=fingerr_ciid WHERE cid=charid;
END IF;

IF melee_ciid <> 0	THEN	-- melee.
UPDATE character SET melee_slot=melee_ciid WHERE cid=charid;
END IF;

IF primary_ciid <> 0	THEN	-- primary.
UPDATE character SET primary_slot=primary_ciid WHERE cid=charid;
END IF;

IF secondary_ciid <> 0	THEN	-- secondary.
UPDATE character SET secondary_slot=secondary_ciid WHERE cid=charid;
END IF;

IF custom1_ciid <> 0	THEN	-- custom 1.
UPDATE character SET custom1_slot=custom1_ciid WHERE cid=charid;
END IF;

IF custom2_ciid <> 0	THEN	-- custom 2.
UPDATE character SET custom2_slot=custom2_ciid WHERE cid=charid;
END IF;

IF avatar_ciid <> 0	THEN	-- avatar.
UPDATE character SET avatar_slot=avatar_ciid WHERE cid=charid;
END IF;

IF community1_ciid <> 0	THEN	-- community 1.
UPDATE character SET community1_slot=community1_ciid WHERE cid=charid;
END IF;

IF community2_ciid <> 0	THEN	-- community 2.
UPDATE character SET community2_slot=community2_ciid WHERE cid=charid;
END IF;

IF longbuff1_ciid <> 0	THEN	-- longbuff 1.
UPDATE character SET longbuff1_slot=longbuff1_ciid WHERE cid=charid;
END IF;

IF longbuff2_ciid <> 0	THEN	-- longbuff 2.
UPDATE character SET longbuff2_slot=longbuff2_ciid WHERE cid=charid;
END IF;

END;$$;


--
-- TOC entry 243 (class 1255 OID 16661)
-- Name: insertcqrecord(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertcqrecord(aid_ integer, scenarioid_ integer, time_ integer) RETURNS void
    LANGUAGE sql
    AS $$INSERT INTO challengequestrecord (aid, scenarioid, time) VALUES (aid_, scenarioid_, time_);$$;


--
-- TOC entry 279 (class 1255 OID 17082)
-- Name: insertgambleitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertgambleitem(cid_ integer, itemid_ integer, count_ integer) RETURNS integer
    LANGUAGE sql
    AS $$INSERT INTO chargambleitem (cid, itemid, quantity) VALUES (cid_, itemid_, count_) RETURNING cgiid;$$;


--
-- TOC entry 255 (class 1255 OID 16662)
-- Name: insertgambleitem(integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertgambleitem(cid_ integer, itemid_ integer, count_ integer, newbp_ integer) RETURNS integer
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT insertgambleitem(cid_, itemid_, count_);$$;


--
-- TOC entry 275 (class 1255 OID 17078)
-- Name: insertitem(integer, integer, integer, timestamp with time zone); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertitem(cid_ integer, itemid_ integer, count_ integer, rentperiod_ timestamp with time zone) RETURNS integer
    LANGUAGE sql
    AS $$INSERT INTO characteritem (cid, itemid, quantity, expiredate) 
VALUES (cid_, itemid_, count_, rentperiod_) RETURNING ciid;$$;


--
-- TOC entry 254 (class 1255 OID 16663)
-- Name: insertitem(integer, integer, integer, timestamp with time zone, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION insertitem(cid_ integer, itemid_ integer, count_ integer, rentperiod_ timestamp with time zone, newbp_ integer) RETURNS integer
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT insertitem(cid_, itemid_, count_, rentperiod_);$$;


--
-- TOC entry 244 (class 1255 OID 16664)
-- Name: ischaracterexists(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION ischaracterexists(searchname character varying) RETURNS integer
    LANGUAGE sql
    AS $$SELECT CASE EXISTS(
   SELECT 1 FROM character WHERE name ILIKE searchname LIMIT 1
) WHEN TRUE THEN 1 ELSE 0 END;$$;


--
-- TOC entry 245 (class 1255 OID 16665)
-- Name: isclanexists(character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION isclanexists(clanname_ character varying) RETURNS integer
    LANGUAGE sql
    AS $$SELECT CASE EXISTS(
   SELECT 1 FROM clan WHERE name ILIKE clanname_ LIMIT 1
) WHEN TRUE THEN 1 ELSE 0 END;$$;


--
-- TOC entry 246 (class 1255 OID 16666)
-- Name: isipbanned(inet); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION isipbanned(ip inet) RETURNS integer
    LANGUAGE sql
    AS $$SELECT CASE EXISTS(
   SELECT 1 FROM bannediplist WHERE addr = ip AND 
   (period > CURRENT_TIMESTAMP OR period IS NULL) LIMIT 1)
WHEN TRUE THEN 1 ELSE 0 END;$$;


--
-- TOC entry 247 (class 1255 OID 16667)
-- Name: itemspend(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION itemspend(ciid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE characteritem SET cid = NULL WHERE ciid = ciid_;$$;


--
-- TOC entry 248 (class 1255 OID 16668)
-- Name: joinclan(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION joinclan(clid_ integer, cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$INSERT INTO clanmember (clid, cid, grade) VALUES (clid_, cid_, 9 /* clan member grade. */);$$;


--
-- TOC entry 249 (class 1255 OID 16669)
-- Name: leaveclan(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION leaveclan(clid_ integer, cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$DELETE FROM clanmember WHERE clid = clid_ AND cid = cid_;$$;


--
-- TOC entry 250 (class 1255 OID 16670)
-- Name: makeperiod(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION makeperiod(hour integer) RETURNS timestamp with time zone
    LANGUAGE sql
    AS $$SELECT CURRENT_TIMESTAMP + CAST((hour || ' hour') as interval);$$;


--
-- TOC entry 281 (class 1255 OID 17085)
-- Name: moveaitemtocitem(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION moveaitemtocitem(aiid_ integer, cid_ integer) RETURNS integer
    LANGUAGE sql
    AS $$UPDATE accountitem SET aid = NULL WHERE aiid = aiid_;

INSERT INTO characteritem (cid, itemid, expiredate, quantity) 
SELECT cid_, itemid, expiredate, quantity FROM accountitem WHERE aiid = aiid_ 
RETURNING ciid;$$;


--
-- TOC entry 287 (class 1255 OID 17097)
-- Name: moveaitemtocitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION moveaitemtocitem(aiid_ integer, cid_ integer, count_ integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE 

  itemid_ integer DEFAULT NULL;
  quantity_a integer DEFAULT NULL;

  ciid_ integer DEFAULT NULL;
  quantity_c integer DEFAULT NULL;

  result_ integer DEFAULT NULL;

BEGIN

-- get existing acc item info.
SELECT itemid, quantity FROM accountitem WHERE aiid = aiid_ INTO itemid_, quantity_a;

IF itemid_ IS NULL
THEN
  RETURN NULL;
END IF;

-- get existing char item info (from item id).
SELECT ciid, quantity FROM characteritem WHERE itemid = itemid_ INTO ciid_, quantity_c;

IF ciid_ IS NULL
THEN
  -- insert as a new item.
  INSERT INTO characteritem (cid, itemid, expiredate, quantity) 
  SELECT cid_, itemid, expiredate, count_ FROM accountitem WHERE aiid = aiid_ 
  RETURNING ciid INTO result_;
ELSE
  -- update as an existing item.
  UPDATE characteritem SET quantity = (quantity_c + count_) WHERE ciid = ciid_;
  result_ := ciid_;
END IF;

-- update acc item quantity.
IF (quantity_a - count_) <= 0
THEN
  UPDATE accountitem SET aid = NULL WHERE aiid = aiid_;
ELSE
  UPDATE accountitem SET quantity = (quantity_a - count_) WHERE aiid = aiid_;
END IF;

RETURN result_;

END;$$;


--
-- TOC entry 286 (class 1255 OID 17095)
-- Name: movecitemtoaitem(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION movecitemtoaitem(ciid_ integer, aid_ integer) RETURNS integer
    LANGUAGE sql
    AS $$UPDATE characteritem SET cid = NULL WHERE ciid = ciid_;

INSERT INTO accountitem (aid, itemid, expiredate, quantity) 
SELECT aid_, itemid, expiredate, quantity FROM characteritem WHERE ciid = ciid_ 
RETURNING aiid;$$;


--
-- TOC entry 288 (class 1255 OID 17098)
-- Name: movecitemtoaitem(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION movecitemtoaitem(ciid_ integer, aid_ integer, count_ integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE 

  itemid_ integer DEFAULT NULL;
  quantity_c integer DEFAULT NULL;

  aiid_ integer DEFAULT NULL;
  quantity_a integer DEFAULT NULL;

  result_ integer DEFAULT NULL;

BEGIN

-- get existing char item info.
SELECT itemid, quantity FROM characteritem WHERE ciid = ciid_ INTO itemid_, quantity_c;

IF itemid_ IS NULL
THEN
  RETURN NULL;
END IF;

-- get existing acc item info (from item id).
SELECT aiid, quantity FROM accountitem WHERE itemid = itemid_ INTO aiid_, quantity_a;

IF aiid_ IS NULL
THEN
  -- insert as a new item.
  INSERT INTO accountitem (aid, itemid, expiredate, quantity) 
  SELECT aid_, itemid, expiredate, count_ FROM characteritem WHERE ciid = ciid_ 
  RETURNING aiid INTO result_;
ELSE
  -- update as an existing item.
  UPDATE accountitem SET quantity = (quantity_a + count_) WHERE aiid = aiid_;
  result_ := aiid_;
END IF;

-- update char item quantity.
IF (quantity_c - count_) <= 0
THEN
  UPDATE characteritem SET cid = NULL WHERE ciid = ciid_;
ELSE
  UPDATE characteritem SET quantity = (quantity_c - count_) WHERE ciid = ciid_;
END IF;

RETURN result_;

END;$$;


--
-- TOC entry 251 (class 1255 OID 16671)
-- Name: removeallequippeditem(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION removeallequippeditem(cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET 
	head_slot = NULL, 
	chest_slot = NULL, 
	hands_slot = NULL, 
	legs_slot = NULL, 
	feet_slot = NULL, 
	fingerl_slot = NULL, 
	fingerr_slot = NULL, 
	melee_slot = NULL, 
	primary_slot = NULL, 
	secondary_slot = NULL, 
	custom1_slot = NULL, 
	custom2_slot = NULL, 
	avatar_slot = NULL, 
	community1_slot = NULL, 
	community2_slot = NULL, 
	longbuff1_slot = NULL, 
	longbuff2_slot = NULL
WHERE cid = cid_;$$;


--
-- TOC entry 252 (class 1255 OID 16672)
-- Name: removeequippeditem(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION removeequippeditem(cid_ integer, slot_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

CASE slot_ 
    WHEN 0 THEN
        UPDATE character SET head_slot = NULL WHERE cid = cid_;
    WHEN 1 THEN
        UPDATE character SET chest_slot = NULL WHERE cid = cid_;
    WHEN 2 THEN
        UPDATE character SET hands_slot = NULL WHERE cid = cid_;
    WHEN 3 THEN
        UPDATE character SET legs_slot = NULL WHERE cid = cid_;
    WHEN 4 THEN
        UPDATE character SET feet_slot = NULL WHERE cid = cid_;
    WHEN 5 THEN
        UPDATE character SET fingerl_slot = NULL WHERE cid = cid_;
    WHEN 6 THEN
        UPDATE character SET fingerr_slot = NULL WHERE cid = cid_;
    WHEN 7 THEN
        UPDATE character SET melee_slot = NULL WHERE cid = cid_;
    WHEN 8 THEN
        UPDATE character SET primary_slot = NULL WHERE cid = cid_;
    WHEN 9 THEN
        UPDATE character SET secondary_slot = NULL WHERE cid = cid_;
    WHEN 10 THEN
        UPDATE character SET custom1_slot = NULL WHERE cid = cid_;
    WHEN 11 THEN
        UPDATE character SET custom2_slot = NULL WHERE cid = cid_;
    WHEN 12 THEN
        UPDATE character SET avatar_slot = NULL WHERE cid = cid_;
    WHEN 13 THEN
        UPDATE character SET community1_slot = NULL WHERE cid = cid_;
    WHEN 14 THEN
        UPDATE character SET community2_slot = NULL WHERE cid = cid_;
    WHEN 15 THEN
        UPDATE character SET longbuff1_slot = NULL WHERE cid = cid_;
    WHEN 16 THEN
        UPDATE character SET longbuff2_slot = NULL WHERE cid = cid_;
ELSE END CASE;

END;$$;


--
-- TOC entry 253 (class 1255 OID 16673)
-- Name: removefriend(integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION removefriend(id_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE friend SET deleteflag = TRUE WHERE id = id_;$$;


--
-- TOC entry 296 (class 1255 OID 17133)
-- Name: sendgift(character varying, character varying, character varying, integer, integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION sendgift(receiver_ character varying, sender_ character varying, message_ character varying, itemid1_ integer, itemid2_ integer, itemid3_ integer, itemid4_ integer, itemid5_ integer, renthourperiod_ integer, quantity_ integer) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
   targetcid_ integer DEFAULT NULL;
   
BEGIN
   SELECT cid FROM character WHERE name ILIKE receiver_ LIMIT 1 INTO targetcid_;
   IF (targetcid_ IS NULL)
   THEN
      RETURN -1; -- error : character not found.
   END IF;

   IF itemid1_ <> 0 THEN PERFORM insertitem(targetcid_, itemid1_, quantity_, makeperiod(renthourperiod_)); END IF;
   IF itemid2_ <> 0 THEN PERFORM insertitem(targetcid_, itemid2_, quantity_, makeperiod(renthourperiod_)); END IF;
   IF itemid3_ <> 0 THEN PERFORM insertitem(targetcid_, itemid3_, quantity_, makeperiod(renthourperiod_)); END IF;
   IF itemid4_ <> 0 THEN PERFORM insertitem(targetcid_, itemid4_, quantity_, makeperiod(renthourperiod_)); END IF;
   IF itemid5_ <> 0 THEN PERFORM insertitem(targetcid_, itemid5_, quantity_, makeperiod(renthourperiod_)); END IF;

   INSERT INTO gift (cid, sender, message, itemid, renthourperiod) VALUES 
   (targetcid_, sender_, message_, ARRAY [itemid1_, itemid2_, itemid3_, itemid4_, itemid5_], renthourperiod_);

   RETURN 0;
END;$$;


--
-- TOC entry 294 (class 1255 OID 17118)
-- Name: setaccountcash(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION setaccountcash(aid_ integer, cash_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE account SET cash = cash_ WHERE aid = aid_;$$;


--
-- TOC entry 291 (class 1255 OID 17102)
-- Name: setblitzscore(integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION setblitzscore(cid_ integer, win_ integer, lose_ integer, point_ integer, medal_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

IF NOT EXISTS (SELECT 1 FROM blitzkrieg WHERE cid = cid_ LIMIT 1)
THEN
   INSERT INTO blitzkrieg (cid, win, lose, point, medal) 
   VALUES (cid_, win_, lose_, point_, medal_);
ELSE
   UPDATE blitzkrieg SET win = win_, lose = lose_, point = point_, medal = medal_ 
   WHERE cid = cid_;
END IF;

END;$$;


--
-- TOC entry 266 (class 1255 OID 16674)
-- Name: setdtscore(integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION setdtscore(cid_ integer, tp_ integer, win_ integer, lose_ integer, finalwin_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

IF NOT EXISTS(SELECT 1 FROM dueltournament WHERE cid = cid_ LIMIT 1) 
THEN
     INSERT INTO dueltournament (cid, tp, win, lose, finalwin) 
          VALUES (cid_, tp_, win_, lose_, finalwin_);
ELSE
     UPDATE dueltournament SET 
          tp = tp_, win = win_, lose = lose_, finalwin = finalwin_ 
        WHERE cid = cid_;
END IF;

END;$$;


--
-- TOC entry 256 (class 1255 OID 16675)
-- Name: setexp(integer, integer, integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION setexp(cid_ integer, lv_ integer, xp_ integer, bp_ integer, kill_ integer, death_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET level = lv_, exp = xp_, bounty = bp_, killcount = kill_, deathcount = death_ WHERE cid = cid_;$$;


--
-- TOC entry 271 (class 1255 OID 17065)
-- Name: setsurvivalpoint(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION setsurvivalpoint(cid_ integer, point_ integer) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN

IF NOT EXISTS(SELECT 1 FROM survival WHERE cid = cid_ LIMIT 1)
THEN
   INSERT INTO survival (cid, point) VALUES (cid_, point_);
ELSE
   UPDATE survival SET point = point_ WHERE cid = cid_;
END IF;

END;$$;


--
-- TOC entry 257 (class 1255 OID 16676)
-- Name: updatebounty(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatebounty(cid_ integer, bp_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET bounty = bp_ WHERE cid = cid_;$$;


--
-- TOC entry 258 (class 1255 OID 16677)
-- Name: updateclanmaster(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateclanmaster(clid_ integer, cid_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE clan SET mastercid = cid_ WHERE clid = clid_;$$;


--
-- TOC entry 259 (class 1255 OID 16678)
-- Name: updateclanmembergrade(integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateclanmembergrade(clid_ integer, cid_ integer, grade_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE clanmember SET grade = grade_ WHERE clid = clid_ AND cid = cid_;$$;


--
-- TOC entry 260 (class 1255 OID 16679)
-- Name: updatedtdailyranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatedtdailyranking() RETURNS void
    LANGUAGE sql
    AS $$UPDATE dueltournament SET 

-- update dt char ranking.
ranking = sq.rk, 
rankingdiff = CASE 
   WHEN (ranking IS NULL) OR (ranking = 0) THEN sq.rk 
   ELSE ranking - sq.rk END

-- fetch ranking.
FROM (SELECT 
         dtrid, 
         ROW_NUMBER() 
           OVER(ORDER BY tp DESC, win DESC, finalwin DESC) 
         AS rk FROM dueltournament
) AS sq 

WHERE dueltournament.dtrid = sq.dtrid;$$;


--
-- TOC entry 293 (class 1255 OID 16680)
-- Name: updatedtweeklyranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatedtweeklyranking() RETURNS void
    LANGUAGE sql
    AS $$UPDATE dueltournament SET 

-- move curr info to prev info & reset curr info.
tp = 1000, 
win = 0, 
lose = 0, 
finalwin = 0, 
ranking = 0, 
tpprev = tp, 
winprev = win, 
loseprev = lose, 
finalwinprev = finalwin, 
rankingprev = ranking, 
-- rank diff (IS NULL) OR (= 0) is not valid.
rankingdiff = CASE 
   WHEN (ranking IS NULL) OR (ranking = 0) THEN sq.rk 
   ELSE ranking - sq.rk END, 
-- update class based on their match score.
class = CASE 
   WHEN (tp <= 1000) OR (win <= 0 AND lose <= 0) THEN 10 
   WHEN sq.rk <= 7 THEN 1 
   WHEN sq.rk <= 15 THEN 2 
   WHEN sq.rk <= 24 THEN 3 
   WHEN sq.rk <= 34 THEN 4 
   WHEN sq.rk <= 45 THEN 5 
   WHEN sq.rk <= 57 THEN 6 
   WHEN sq.rk <= 70 THEN 7 
   WHEN sq.rk <= 84 THEN 8 
   WHEN sq.rk <= 99 THEN 9 
   ELSE 10 END

-- fetch ranking.
FROM (SELECT 
         dtrid, 
         ROW_NUMBER() 
           OVER(ORDER BY tp DESC, win DESC, finalwin DESC) 
         AS rk FROM dueltournament
) AS sq 

WHERE dueltournament.dtrid = sq.dtrid;$$;


--
-- TOC entry 278 (class 1255 OID 17081)
-- Name: updategambleitem(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updategambleitem(cgiid_ integer, count_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE chargambleitem SET quantity = count_ WHERE cgiid = cgiid_;$$;


--
-- TOC entry 268 (class 1255 OID 16681)
-- Name: updategambleitem(integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updategambleitem(cid_ integer, cgiid_ integer, count_ integer, newbp_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT updategambleitem(cgiid_, count_);$$;


--
-- TOC entry 261 (class 1255 OID 16682)
-- Name: updateindividualranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateindividualranking() RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET ranking = rk FROM ( 
SELECT cid, RANK() OVER(ORDER BY exp DESC, killcount DESC) AS rk 
FROM character WHERE deleteflag = FALSE 
) AS sq WHERE character.cid = sq.cid;$$;


--
-- TOC entry 277 (class 1255 OID 17080)
-- Name: updateitem(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateitem(ciid_ integer, count_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE characteritem SET quantity = count_ WHERE ciid = ciid_;$$;


--
-- TOC entry 267 (class 1255 OID 16683)
-- Name: updateitem(integer, integer, integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateitem(cid_ integer, ciid_ integer, count_ integer, newbp_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET bounty = newbp_ WHERE cid = cid_;
SELECT updateitem(ciid_, count_);$$;


--
-- TOC entry 262 (class 1255 OID 16684)
-- Name: updateitemquantity(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateitemquantity(ciid_ integer, count_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE characteritem SET quantity = count_ WHERE ciid = ciid_;$$;


--
-- TOC entry 263 (class 1255 OID 16685)
-- Name: updatelastconndata(integer, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatelastconndata(aid_ integer, ip_ character varying) RETURNS void
    LANGUAGE sql
    AS $$UPDATE account SET lastconndate = CURRENT_TIMESTAMP, lastip = ip_ WHERE aid = aid_;$$;


--
-- TOC entry 264 (class 1255 OID 16686)
-- Name: updatequestitem(integer, character varying); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatequestitem(cid_ integer, data_ character varying) RETURNS void
    LANGUAGE sql
    AS $$UPDATE character SET questiteminfo = data_ WHERE cid = cid_;$$;


--
-- TOC entry 265 (class 1255 OID 16687)
-- Name: updateserverstatus(integer, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updateserverstatus(serverid_ integer, currplayer_ integer) RETURNS void
    LANGUAGE sql
    AS $$UPDATE serverstatus SET currplayer = currplayer_, time = CURRENT_TIMESTAMP WHERE id = serverid_;$$;


--
-- TOC entry 274 (class 1255 OID 17064)
-- Name: updatesurvivalranking(); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION updatesurvivalranking() RETURNS void
    LANGUAGE sql
    AS $$UPDATE survival SET ranking = sq.rk FROM (
  SELECT id, RANK() OVER(ORDER BY point DESC) AS rk FROM survival
) AS sq WHERE survival.id = sq.id;$$;


--
-- TOC entry 174 (class 1259 OID 16688)
-- Name: account_aid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE account_aid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2191 (class 0 OID 0)
-- Dependencies: 174
-- Name: account_aid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE account_aid_seq OWNED BY account.aid;


--
-- TOC entry 196 (class 1259 OID 17070)
-- Name: accountitem; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE accountitem (
    aiid integer NOT NULL,
    aid integer,
    itemid integer NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    expiredate timestamp with time zone,
    quantity integer DEFAULT 1 NOT NULL
);


--
-- TOC entry 195 (class 1259 OID 17068)
-- Name: accountitem_aiid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE accountitem_aiid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2192 (class 0 OID 0)
-- Dependencies: 195
-- Name: accountitem_aiid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE accountitem_aiid_seq OWNED BY accountitem.aiid;


--
-- TOC entry 175 (class 1259 OID 16690)
-- Name: bannediplist; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE bannediplist (
    id integer NOT NULL,
    addr inet NOT NULL,
    period timestamp with time zone
);


--
-- TOC entry 176 (class 1259 OID 16696)
-- Name: bannediplist_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE bannediplist_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2193 (class 0 OID 0)
-- Dependencies: 176
-- Name: bannediplist_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE bannediplist_id_seq OWNED BY bannediplist.id;


--
-- TOC entry 189 (class 1259 OID 16779)
-- Name: blitzkrieg_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE blitzkrieg_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2194 (class 0 OID 0)
-- Dependencies: 189
-- Name: blitzkrieg_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE blitzkrieg_id_seq OWNED BY blitzkrieg.id;


--
-- TOC entry 191 (class 1259 OID 16791)
-- Name: blitzshop_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE blitzshop_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2195 (class 0 OID 0)
-- Dependencies: 191
-- Name: blitzshop_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE blitzshop_id_seq OWNED BY blitzshop.id;


--
-- TOC entry 177 (class 1259 OID 16698)
-- Name: challengequestrecord; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE challengequestrecord (
    id integer NOT NULL,
    aid integer,
    scenarioid integer NOT NULL,
    "time" integer NOT NULL
);


--
-- TOC entry 178 (class 1259 OID 16701)
-- Name: challengequestrecord_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE challengequestrecord_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2196 (class 0 OID 0)
-- Dependencies: 178
-- Name: challengequestrecord_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE challengequestrecord_id_seq OWNED BY challengequestrecord.id;


--
-- TOC entry 179 (class 1259 OID 16703)
-- Name: character_cid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE character_cid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2197 (class 0 OID 0)
-- Dependencies: 179
-- Name: character_cid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE character_cid_seq OWNED BY "character".cid;


--
-- TOC entry 180 (class 1259 OID 16705)
-- Name: characteritem; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE characteritem (
    ciid integer NOT NULL,
    cid integer,
    itemid integer NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL,
    expiredate timestamp with time zone,
    quantity integer DEFAULT 1 NOT NULL
);


--
-- TOC entry 181 (class 1259 OID 16710)
-- Name: characteritem_ciid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE characteritem_ciid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2198 (class 0 OID 0)
-- Dependencies: 181
-- Name: characteritem_ciid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE characteritem_ciid_seq OWNED BY characteritem.ciid;


--
-- TOC entry 182 (class 1259 OID 16712)
-- Name: chargambleitem_cgiid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE chargambleitem_cgiid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2199 (class 0 OID 0)
-- Dependencies: 182
-- Name: chargambleitem_cgiid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE chargambleitem_cgiid_seq OWNED BY chargambleitem.cgiid;


--
-- TOC entry 183 (class 1259 OID 16714)
-- Name: clan_clid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE clan_clid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2200 (class 0 OID 0)
-- Dependencies: 183
-- Name: clan_clid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE clan_clid_seq OWNED BY clan.clid;


--
-- TOC entry 184 (class 1259 OID 16716)
-- Name: clanmember; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE clanmember (
    cmid integer NOT NULL,
    clid integer NOT NULL,
    cid integer,
    grade integer NOT NULL,
    regdate timestamp with time zone DEFAULT now() NOT NULL
);


--
-- TOC entry 185 (class 1259 OID 16720)
-- Name: clanmember_cmid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE clanmember_cmid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2201 (class 0 OID 0)
-- Dependencies: 185
-- Name: clanmember_cmid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE clanmember_cmid_seq OWNED BY clanmember.cmid;


--
-- TOC entry 186 (class 1259 OID 16722)
-- Name: dueltournament_dtrid_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE dueltournament_dtrid_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2202 (class 0 OID 0)
-- Dependencies: 186
-- Name: dueltournament_dtrid_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE dueltournament_dtrid_seq OWNED BY dueltournament.dtrid;


--
-- TOC entry 187 (class 1259 OID 16724)
-- Name: friend; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE friend (
    id integer NOT NULL,
    cid integer NOT NULL,
    friendcid integer NOT NULL,
    deleteflag boolean DEFAULT false NOT NULL
);


--
-- TOC entry 188 (class 1259 OID 16728)
-- Name: friend_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE friend_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2203 (class 0 OID 0)
-- Dependencies: 188
-- Name: friend_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE friend_id_seq OWNED BY friend.id;


--
-- TOC entry 198 (class 1259 OID 17121)
-- Name: gift; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE gift (
    id integer NOT NULL,
    cid integer,
    sender character varying(24) NOT NULL,
    message character varying(128) NOT NULL,
    itemid integer[] NOT NULL,
    renthourperiod integer NOT NULL,
    giftdate timestamp with time zone DEFAULT now() NOT NULL
);


--
-- TOC entry 197 (class 1259 OID 17119)
-- Name: gift_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE gift_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2204 (class 0 OID 0)
-- Dependencies: 197
-- Name: gift_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE gift_id_seq OWNED BY gift.id;


--
-- TOC entry 193 (class 1259 OID 17035)
-- Name: survival_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE survival_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- TOC entry 2205 (class 0 OID 0)
-- Dependencies: 193
-- Name: survival_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE survival_id_seq OWNED BY survival.id;


--
-- TOC entry 2096 (class 2604 OID 16730)
-- Name: aid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY account ALTER COLUMN aid SET DEFAULT nextval('account_aid_seq'::regclass);


--
-- TOC entry 2147 (class 2604 OID 17073)
-- Name: aiid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY accountitem ALTER COLUMN aiid SET DEFAULT nextval('accountitem_aiid_seq'::regclass);


--
-- TOC entry 2128 (class 2604 OID 16731)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY bannediplist ALTER COLUMN id SET DEFAULT nextval('bannediplist_id_seq'::regclass);


--
-- TOC entry 2137 (class 2604 OID 16784)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY blitzkrieg ALTER COLUMN id SET DEFAULT nextval('blitzkrieg_id_seq'::regclass);


--
-- TOC entry 2142 (class 2604 OID 16796)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY blitzshop ALTER COLUMN id SET DEFAULT nextval('blitzshop_id_seq'::regclass);


--
-- TOC entry 2129 (class 2604 OID 16732)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY challengequestrecord ALTER COLUMN id SET DEFAULT nextval('challengequestrecord_id_seq'::regclass);


--
-- TOC entry 2105 (class 2604 OID 16733)
-- Name: cid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY "character" ALTER COLUMN cid SET DEFAULT nextval('character_cid_seq'::regclass);


--
-- TOC entry 2132 (class 2604 OID 16734)
-- Name: ciid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY characteritem ALTER COLUMN ciid SET DEFAULT nextval('characteritem_ciid_seq'::regclass);


--
-- TOC entry 2107 (class 2604 OID 16735)
-- Name: cgiid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY chargambleitem ALTER COLUMN cgiid SET DEFAULT nextval('chargambleitem_cgiid_seq'::regclass);


--
-- TOC entry 2117 (class 2604 OID 16736)
-- Name: clid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY clan ALTER COLUMN clid SET DEFAULT nextval('clan_clid_seq'::regclass);


--
-- TOC entry 2134 (class 2604 OID 16737)
-- Name: cmid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY clanmember ALTER COLUMN cmid SET DEFAULT nextval('clanmember_cmid_seq'::regclass);


--
-- TOC entry 2127 (class 2604 OID 16738)
-- Name: dtrid; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY dueltournament ALTER COLUMN dtrid SET DEFAULT nextval('dueltournament_dtrid_seq'::regclass);


--
-- TOC entry 2136 (class 2604 OID 16739)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY friend ALTER COLUMN id SET DEFAULT nextval('friend_id_seq'::regclass);


--
-- TOC entry 2150 (class 2604 OID 17124)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY gift ALTER COLUMN id SET DEFAULT nextval('gift_id_seq'::regclass);


--
-- TOC entry 2145 (class 2604 OID 17040)
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY survival ALTER COLUMN id SET DEFAULT nextval('survival_id_seq'::regclass);


--
-- TOC entry 2153 (class 2606 OID 16741)
-- Name: account_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY account
    ADD CONSTRAINT account_pkey PRIMARY KEY (aid);


--
-- TOC entry 2181 (class 2606 OID 17077)
-- Name: accountitem_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY accountitem
    ADD CONSTRAINT accountitem_pkey PRIMARY KEY (aiid);


--
-- TOC entry 2165 (class 2606 OID 16743)
-- Name: bannediplist_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY bannediplist
    ADD CONSTRAINT bannediplist_pkey PRIMARY KEY (id);


--
-- TOC entry 2175 (class 2606 OID 16790)
-- Name: blitzkrieg_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY blitzkrieg
    ADD CONSTRAINT blitzkrieg_pkey PRIMARY KEY (id);


--
-- TOC entry 2177 (class 2606 OID 16800)
-- Name: blitzshop_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY blitzshop
    ADD CONSTRAINT blitzshop_pkey PRIMARY KEY (id);


--
-- TOC entry 2167 (class 2606 OID 16745)
-- Name: challengequestrecord_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY challengequestrecord
    ADD CONSTRAINT challengequestrecord_pkey PRIMARY KEY (id);


--
-- TOC entry 2155 (class 2606 OID 16747)
-- Name: character_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY "character"
    ADD CONSTRAINT character_pkey PRIMARY KEY (cid);


--
-- TOC entry 2169 (class 2606 OID 16749)
-- Name: characteritem_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY characteritem
    ADD CONSTRAINT characteritem_pkey PRIMARY KEY (ciid);


--
-- TOC entry 2157 (class 2606 OID 16751)
-- Name: chargambleitem_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY chargambleitem
    ADD CONSTRAINT chargambleitem_pkey PRIMARY KEY (cgiid);


--
-- TOC entry 2159 (class 2606 OID 16753)
-- Name: clan_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY clan
    ADD CONSTRAINT clan_pkey PRIMARY KEY (clid);


--
-- TOC entry 2171 (class 2606 OID 16755)
-- Name: clanmember_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY clanmember
    ADD CONSTRAINT clanmember_pkey PRIMARY KEY (cmid);


--
-- TOC entry 2161 (class 2606 OID 16757)
-- Name: dueltournament_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY dueltournament
    ADD CONSTRAINT dueltournament_pkey PRIMARY KEY (dtrid);


--
-- TOC entry 2173 (class 2606 OID 16759)
-- Name: friend_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY friend
    ADD CONSTRAINT friend_pkey PRIMARY KEY (id);


--
-- TOC entry 2183 (class 2606 OID 17130)
-- Name: gift_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY gift
    ADD CONSTRAINT gift_pkey PRIMARY KEY (id);


--
-- TOC entry 2163 (class 2606 OID 16761)
-- Name: serverstatus_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY serverstatus
    ADD CONSTRAINT serverstatus_pkey PRIMARY KEY (id);


--
-- TOC entry 2179 (class 2606 OID 17043)
-- Name: survival_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY survival
    ADD CONSTRAINT survival_pkey PRIMARY KEY (id);


-- Completed on 2013-08-31 06:29:00

--
-- PostgreSQL database dump complete
--

