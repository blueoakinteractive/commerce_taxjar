<?php

/**
 * @file
 * Class file to interface with TaxJar API.
 */
use TaxJar\Client;

class CommerceTaxJar extends Client {
  protected $wrapper;
  protected $errors;

  private $parameters;
  private $settings;

  /**
   * Instantiates a new TaxJar Client class.
   *
   * @param $settings
   * @throws \Exception
   *  Thrown with the required API Key is not passed as an argument.
   * @return \CommerceTaxJar
   */
  public static function withSettings($settings) {
    if (empty($settings['api_key'])) {
      throw new \Exception('TaxJar API instantiated without required API Key');
    }
    return new self($settings);
  }

  /**
   * Constructs a a new \CommerceTaxJar object.
   * @param $settings
   *   Array of settings saved by the commerce_taxjar settings form.
   */
  public function __construct($settings) {
    parent::__construct($settings['api_key']);
    $this->settings = $settings;
  }

  /**
   * Store the entity metadata wrapper as a property of this object.
   *
   * @param \EntityDrupalWrapper $wrapper
   */
  public function setWrapper($wrapper) {
    $this->wrapper = $wrapper;
  }

  /**
   * Fetch taxes for an order.
   */
  public function getTax() {
    $this->initParameters()
      ->setTo()
      ->setFrom()
      ->setSubTotal()
      ->setShipping()
      ->setLineItems();

    if (empty($this->getErrors())) {
      try {
        $tax = $this->taxForOrder($this->parameters);
      } catch (\TaxJar\Util\Exception $ex) {
        // Unable to fetch tax data at this time.
      }
      if (!empty($tax)) {
        $this->setTax($tax);
      }
    }
  }

  /**
   * Returns a line item wrapper matching the line item id.
   *
   * @param $line_item_id
   * @return mixed
   */
  private function getLineItemWrapperById($line_item_id) {
    foreach ($this->wrapper->commerce_line_items as $line_item_wrapper) {
      if ($line_item_wrapper->line_item_id->value() == $line_item_id) {
        return $line_item_wrapper;
      }
    }
    return NULL;
  }

  /**
   * Creates tax price components per line item from the TaxJar response.
   *
   * @param $tax
   */
  protected function setTax($tax) {
    foreach ($tax->breakdown->line_items as $line_item) {
      $line_item_id = $line_item->id;
      try {
        $line_item_wrapper = $this->getLineItemWrapperById($line_item_id);
        $price = $line_item_wrapper->commerce_unit_price->value();
        $currency_code = $price['currency_code'];

        $taxjar_price = array(
          'amount' => commerce_currency_decimal_to_amount($line_item->tax_collectable, $currency_code),
          'currency_code' => $currency_code,
        );
        $price['data'] = commerce_price_component_delete($price, 'taxjar_tax');
        $price['data'] = commerce_price_component_add($price, 'taxjar_tax', $taxjar_price, FALSE);
        $line_item_wrapper->commerce_unit_price->set($price);
        $line_item_wrapper->save();
      }
      catch (EntityMetadataWrapperException $ex) {
        watchdog('commerce_taxjar', 'Unable to set tax for line item @line_item_id', array('@line_item_id' => $line_item_id));
      }
    }
  }

  /**
   * Initialize the parameters for a TaxJar API request.
   */
  private function initParameters() {
    $this->parameters = new \stdClass();
    return $this;
  }

  /**
   * Set "to" values for a TaxJar API request.
   *
   * @return $this
   */
  private function setTo() {
    try {
      $address = $this->wrapper->commerce_customer_shipping->commerce_customer_address->value();
      $this->parameters->to_country = $address['country'];
      $this->parameters->to_zip = $address['postal_code'];
      $this->parameters->to_state = $address['administrative_area'];
      $this->parameters->to_street = $address['thoroughfare'];
    } catch (EntityMetadataWrapperException $ex) {
      $this->setError(t('Unable to set destination address'));
    }
    return $this;
  }

  /**
   * Set "from" values for a TaxJar API request.
   *
   * @return $this
   */
  private function setFrom() {
    // @todo: Add values for setting a from address.
    return $this;
  }

  /**
   * Set amount value for a TaxJar API request.
   *
   * @return $this
   */
  private function setSubTotal() {
    try {
      $total = $this->wrapper->commerce_order_total->value();
      $this->parameters->amount = commerce_currency_amount_to_decimal($total['amount'], $total['currency_code']);
    } catch (EntityMetadataWrapperException $ex) {
      $this->setError(t('Unable to set order total'));
    }
    return $this;
  }

  /**
   * Set shipping value for a TaxJar API request.
   *
   * @return $this
   */
  private function setShipping() {
    $shipping_total = 0;

    if (!empty($this->wrapper->commerce_line_items)) {
      try {
        foreach ($this->wrapper->commerce_line_items as $line_item_wrapper) {
          if ($line_item_wrapper->type->value() == 'shipping') {
            $total = $line_item_wrapper->commerce_total->value();
            $shipping_total = commerce_currency_decimal_to_amount($total['amount'], $total['currency_code']);
          }
        }
      } catch (EntityMetadataWrapperException $ex) {
        // Exceptions here are not critical.
      }
    }
    else {
      $this->setError(t('Unable to access commerce line items for shipping information.'));
    }

    $this->parameters->shipping = $shipping_total;
    return $this;
  }

  /**
   * Set line item values for a TaxJar API request.
   *
   * @return $this
   */
  private function setLineItems() {
    $line_items = array();
    if (!empty($this->wrapper->commerce_line_items)) {
      // Attempt to loop over the order line items.
      try {
        foreach ($this->wrapper->commerce_line_items as $line_item_wrapper) {
          $line_item = new \stdClass();
          if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())) {
            $line_item->quantity = $line_item_wrapper->quantity->value();
            $line_item->id = $line_item_wrapper->line_item_id->value();

            // Load the unit price excluding any TaxJar tax components.
            $unit_price = $line_item_wrapper->commerce_unit_price->value();
            $unit_price['data'] = commerce_price_component_delete($unit_price, 'taxjar_tax');
            $unit_price = commerce_price_component_total($unit_price);
            $line_item->unit_price = commerce_currency_amount_to_decimal($unit_price['amount'], $unit_price['currency_code']);

            // Attempt to access the line item product data.
            try {
              $product_wrapper = $line_item_wrapper->commerce_product;
              $line_item->product_identifier = $product_wrapper->sku->value();

              // Attempt to access the product tax category.
              try {
                $line_item->product_tax_code = $product_wrapper->product_taxjar_category->value();
              } catch (EntityMetadataWrapperException $tax_category_exception) {

                // Attempt to access the default product tax category if the product doesn't
                // have one set.
                $type = $product_wrapper->type->value();
                if (!empty($this->settings['default_tax_category'][$type])) {
                  $line_item->product_tax_code = $this->settings['default_tax_category'][$type];
                }
                else {
                  // Set an error if we're still unable to determine the product tax category.
                  $this->setError(t('Unable to set the TaxJar category for line item @line_item_id', array('@line_item_id' => $line_item_wrapper->line_item_id->value())));
                }
              }
            } catch (EntityMetadataWrapperException $product_exception) {
              // Only append line item data for product based line items.
              continue;
            }
          }

          // Append the current line item data to the line items array.
          $line_items[] = $line_item;
        }
      } catch (EntityMetadataWrapperException $ex) {
        // Exceptions here are not critical.
      }
    }

    if (!empty($line_items)) {
      $this->parameters->line_items = $line_items;
    }
    else {
      $this->setError(t('Unable to set line items for the order'));
    }

    return $this;
  }

  /**
   * Set an error on this object.
   *
   * @param $error
   *   String value describing the error.
   */
  private function setError($error) {
    $this->errors[] = $error;
  }

  /**
   * Gets any errors encountered when invoking this object.
   *
   * @return mixed
   *   An array of errors.
   */
  public function getErrors() {
    return $this->errors;
  }
}
