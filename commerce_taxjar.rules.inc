<?php
/**
 * @file
 * Provides rules integration for Commerce TaxJar
 */

/**
 * Implements hook_rules_event_info().
 */
function commerce_taxjar_rules_event_info() {
  $events = array();
  $events['commerce_taxjar_refresh_order'] = array(
    'label' => t('Commerce Order refreshed for tax calculation'),
    'group' => t('Commerce Order'),
    'variables' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
      ),
    ),
  );
  return $events;
}

/**
 * Implements hook_rules_action_info().
 */
function commerce_taxjar_rules_action_info() {
  $info = array();
  $info['commerce_taxjar_calculate_tax'] = array(
    'label' => t('Calculate tax via TaxJar'),
    'group' => t('Commerce Order'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'wrapped' => TRUE,
      ),
    ),
    'callbacks' => array(
      'execute' => 'commerce_taxjar_calculate_tax',
    )
  );
  return $info;
}
