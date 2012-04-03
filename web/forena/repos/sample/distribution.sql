--ACCESS=design any report
select * from user_distribution WHERE 
  COALESCE( :state, state) = state 
