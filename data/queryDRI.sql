.output DRI.txt
select n.nutrdesc,l.desc,d.min_age||'<=age<'||d.max_age,'EAR:'||d.ear||n.units,'RDA:'||d.rda||n.units,'AI:'||d.ai||n.units,'UL:'||d.ul||n.units from dri d join life_stage_groups l on d.lsg=l.id join nutr_def n using(nutr_no) order by nutr_no,lsg,min_age;
.output DRIperkg.txt
select n.nutrdesc,l.desc,d.min_age||'<=age<'||d.max_age,'EAR:'||d.ear||n.units,'RDA:'||d.rda||n.units,'AI:'||d.ai||n.units,'UL:'||d.ul||n.units from driperkg d join life_stage_groups l on d.lsg=l.id join nutr_def n using(nutr_no) order by nutr_no,lsg,min_age;
.output stdout
