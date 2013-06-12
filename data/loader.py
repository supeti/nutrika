#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sqlite3

conn = sqlite3.connect('/home/peti/works/nutrika/data/nutrika.db')

"""
CREATE TABLE weights (
ndb_no character varying(5) REFERENCES food_desc(ndb_no), 
seq character varying(2) NOT NULL, 
amount numeric(5,3) NOT NULL, 
measure_desc character varying(80) NOT NULL, 
gm_wgt numeric(7,1) NOT NULL, 
num_data_pts numeric(3,0), 
std_dev numeric(7,3), 
PRIMARY KEY (ndb_no, seq))
"""
conn.execute("DELETE FROM weights;")
f = open('/home/peti/Downloads/diet/sr25/WEIGHT.txt')
for l in f:
  a = l.strip("\r\n").replace("'","''").replace("~","").split('^')
  conn.execute("INSERT INTO weights VALUES ('%s','%s','%s','%s','%s','%s','%s');"%tuple(a))
f.close()
conn.commit()

"""
CREATE TABLE nutr_data (
ndb_no character varying(5) REFERENCES food_desc(ndb_no), 
nutr_no character varying(3) REFERENCES nutr_def(nutr_no), 
nutr_val numeric(10,3) NOT NULL, 
add_nutr_mark character(1))
"""
conn.execute("DELETE FROM nutr_data;")
f = open('/home/peti/Downloads/diet/sr25/NUT_DATA.txt')
for l in f:
  a = l.strip("\r\n").replace("'","''").replace("~","").split('^')
  conn.execute("INSERT INTO nutr_data VALUES ('%s','%s','%s','%s');"%(a[0],a[1],a[2],a[8]))
f.close()
conn.execute("INSERT INTO nutr_data SELECT ndb_no,531,sum(nutr_val),null FROM nutr_data WHERE nutr_no=506 OR nutr_no=507 GROUP BY ndb_no;")
conn.execute("INSERT INTO nutr_data SELECT ndb_no,532,sum(nutr_val),null FROM nutr_data WHERE nutr_no=508 OR nutr_no=509 GROUP BY ndb_no;")
conn.commit()

"""
CREATE TABLE food_desc (
ndb_no character varying(5) PRIMARY KEY, 
fdgrp character varying(4) REFERENCES food_groups(code), 
long_desc character varying(200) NOT NULL, 
shrt_desc character varying(60) NOT NULL, 
comname character varying(100), 
manufacname character varying(65), 
survey character(1), 
ref_desc character varying(135), 
refuse numeric(2,0), 
sciname character varying(65), 
n_factor numeric(4,2), 
pro_factor numeric(4,2), 
fat_factor numeric(4,2), 
cho_factor numeric(4,2))
"""
conn.execute("DELETE FROM food_desc;")
f = open('/home/peti/Downloads/diet/sr25/FOOD_DES.txt', encoding='latin1')
for l in f:
  a = l.strip("\r\n").replace("'","''").replace("~","").split('^')
  conn.execute("INSERT INTO food_desc VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s');"%tuple(a))
f.close()
conn.commit()

conn.close()

