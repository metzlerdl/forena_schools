<?php
class Search extends ForenaSchools {

	public function auth() {
		return $this->access('teacher');
	}




}