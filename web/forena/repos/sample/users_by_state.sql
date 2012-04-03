--ACCESS=design any report
select code AS state,name,total from states s join 
  (select state,sum(users) as total from user_distribution group by state) su
  ON su.state=s.code
ORDER BY total desc

