<?php

/**
 * @file
 * Provides admin settings form for TaxJar.
 */

/**
 * Form callback for TaxJar settings.
 */
function commerce_taxjar_admin_settings_form($form, &$form_state) {
  $defaults = array_fill_keys(commerce_taxjar_settings_values(), NULL);
  $settings = array_merge($defaults, variable_get('commerce_taxjar_settings'));

  if (!empty($settings['api_key'])) {
    $taxjar = commerce_taxjar_api();
  }

  $form['api_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('API setup'),
  );
  $form['api_settings']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TaxJar API key'),
    '#description' => t('Enter the TaxJar API key for your account. !link',
      array(
        '!link' => l(t('TaxJar Account'), 'https://app.taxjar.com/account#api-access', array('attributes' => array('target' => '_blank')))
      )
    ),
    '#default_value' => $settings['api_key'],
    '#required' => TRUE,
  );

  if (!empty($taxjar)) {
    $form['default_tax_category'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => t('Product setup'),
      '#description' => t('Use the options below to configure the default tax category for each '
        . 'product type in your store. If a value is set, a new field instance will be created '
        . 'on the bundle. Note, this will not set a value for existing products or delete the field '
        . 'if the value is un-selected'),
    );

    foreach (commerce_product_type_options_list() as $bundle => $name) {
      $form['default_tax_category'][$bundle] = array(
        '#type' => 'select',
        '#title' => t('Default tax category for "@bundle" products', array('@bundle' => $name)),
        '#options' => commerce_taxjar_categories_list(),
        '#default_value' => !empty($settings['default_tax_category'][$bundle]) ? $settings['default_tax_category'][$bundle] : 0,
      );
    }
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
  // Create field instances for each product bundle set.
  foreach ($form_state['values']['default_tax_category'] as $bundle => $category) {
    if (!empty($category)) {
      commerce_taxjar_create_field_instances($bundle, $category);
    }
  }

  // Save appropriate TaxJar settings as a Drupal variable.
  $settings = array();
  foreach ($form_state['values'] as $key => $value) {
    if (in_array($key, commerce_taxjar_settings_values())) {
      $settings[$key] = $value;
    }
  }

  variable_set('commerce_taxjar_settings', $settings);
  drupal_set_message(t('TaxJar settings saved.'));
}
