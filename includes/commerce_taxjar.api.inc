<?php

/**
 * @file
 * Class file to interface with TaxJar API.
 */
use TaxJar\Client;

class CommerceTaxJar extends Client {
  public function __construct($key) {
    parent::__construct($key);
  }
}
