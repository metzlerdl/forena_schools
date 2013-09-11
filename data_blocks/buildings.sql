--ACCESS=PUBLIC
select bldg_id,name FROM i_buildings 
where bldg_id<>-1
order by name
