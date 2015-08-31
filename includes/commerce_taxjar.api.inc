<?php

/**
 * @file
 * Class file to interface with TaxJar API.
 */
use TaxJar\Client;

class CommerceTaxJar extends Client {
  protected $wrapper;

  /**
   * Overwrite the parent method to return an instance of this class.
   *
   * @param $key
   * @return \CommerceTaxJar
   */
  public static function withApiKey($key) {
    return new self($key);
  }

  /**
   * @inheritdoc
   */
  public function __construct($key) {
    parent::__construct($key);
  }

  /**
   * Store the entity metadata wrapper as a property of this object.
   *
   * @param \EntityDrupalWrapper $wrapper
   */
  public function setWrapper($wrapper) {
    $this->wrapper = $wrapper;
  }
}
