<?php

/**
 * @file
 * Provides admin settings form for TaxJar.
 */

/**
 * Form callback for TaxJar settings.
 */
function commerce_taxjar_admin_settings_form($form, &$form_state) {
  $api_key = variable_get('commerce_taxjar_api_key');

  if (!empty($api_key)) {
    $taxjar = commerce_taxjar_api();
  }

  $form['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TaxJar API key'),
    '#description' => t('Enter the TaxJar API key for your account. !link',
      array(
        '!link' => l(t('TaxJar Account'), 'https://app.taxjar.com/account#api-access', array('attributes' => array('target' => '_blank')))
      )
    ),
    '#default_value' => $api_key,
    '#required' => TRUE,
  );

  $form['product_bundles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Taxable product types'),
    '#description' => t('Select the taxable product types in your store.
      This will create fields on these products so you can specify the tax category.'),
    '#options' => commerce_product_type_options_list(),
  );

  if (!empty($taxjar)) {
    $form['default_tax_category'] = array(
      '#type' => 'select',
      '#title' => t('Default tax category'),
      '#description' => t('Select the default tax category that matches the majority of your products'),
      '#options' => commerce_taxjar_categories_list(),
    );
  }
  else {
    $form['more'] = array(
      '#markup' => '<strong>' . t('Note: More options will be displayed once you have entered your API key.') . '</strong>',
    );
  }


  $form['actions'] = array(
    '#type' => 'container',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save settings'),
    )
  );
  return $form;
}

/**
 * Validation callback for TaxJar settings form.
 */
function commerce_taxjar_admin_settings_form_validate($form, &$form_state) {
  // No validation required currently.
}

/**
 * Submit callback for TaxJar settings form.
 */
function commerce_taxjar_admin_settings_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['product_bundles'])) {
    foreach ($form_state['values']['product_bundles'] as $bundle) {
      // @todo: Create field instances for product tax categories.
    }
  }

  // Save appropriate TaxJar settings as a Drupal variable.
  $settings = array();
  foreach ($form_state['values'] as $key => $value) {
    if (in_array($key, commerce_taxjar_settings_fields())) {
      $settings[$key] = $value;
    }
  }
  variable_set('commerce_taxjar_settings', $settings);
  drupal_set_message(t('TaxJar settings saved.'));
}
