<?php
class FrxData {

	/**
	 * The FrxData class holds all of the data contexts during the report rendering process.
	 * The general idea is that during the report render, data objects are pushed on the stack with the id's of the block or foreach objects that invoke them.
	 * Rendering controls may then get current data contexts, and push data onto the stack or pop data onto the stack. They may also use this method to get the current data
	 * context from the stack.
	 *
	 * The static instance function of this object acts as a factory method that allows other code in forena to always be able to operate on the current context of the class.
	 * @var unknown_type
	 */
	public $id;
	private static $instance='';
	private $cur_context;  // The data of the xml;
	private $data_sources = array();
  private $data_stack = array();
  private $id_stack = array();

	/**
	 * Static factory method for working on data objects
	 * Enter description here ...
	 */
	static function instance() {
		if (!self::$instance) {
			self::$instance = new FrxData();
		}
		return self::$instance;
	}

	/**
	 * Return the current data context
	 *
	 */
	public function currentContext() {
		return $this->cur_context;
	}

	/**
	 * Allows override of a value for the current context.
	 * @param String $key
	 * @param String $value
	 */
  public function setValue($key, $value) {
     if (is_array($this->cur_context)) {
       $this->cur_context[$key] = $value;
     }
     elseif (is_object($parameters)) {
     	 if (strpos($key, '@')===0) {
     	   $this->cur_context[$key] = $value;
     	 }
     	 else {
         $parmeters->$key = $value;
     	 }
     }
  }

  /**
   * Push a data context onto the data stacks
   * to make sure that we can address these using an
   * appropriate syntax.  I think we don't need data_stack
   * but i'm holding it there in case we develop a "relative" path syntax.
   * @param $data
   * @param $id
   * @return unknown_type
   */
  public function push($data, $id='') {
  	$this->data_stack[] = $this->cur_context;
  	$this->id_stack[] = $this->id;
    $this->id = $id;
    $this->cur_context = $data;

    if ($id) {
      $this->data_sources[$id] = $data;
    }
  }

  /**
   * Remove data from the data stack.  This will make data unavaiable
   * when we leave the context of the current nested reports.
   * @param $id
   * @return mixed
   */
  public function pop() {
  	$this->id = array_pop($this->id_stack);
    $this->cur_context = array_pop($this->data_stack);

  }

  /**
   * Determines whether an array context exists for the specified id.
   * Returns true if the key exists othewise false
   * @param string $id
   * @return boolean
   */
  public function contextExists($id) {
  	if (array_key_exists($id, $this->data_sources)){
  		return TRUE;
  	}
  	else return FALSE;
  }

  /**
   * Return a data context by id
   * Enter description here ...
   * @param unknown_type $id
   */
  public function getContext($id) {
  	return @$this->data_sources[$id];
  }

}