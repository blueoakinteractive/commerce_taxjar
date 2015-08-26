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
  variable_set('commerce_taxjar_api_key', $form_state['values']['api_key']);
  drupal_set_message(t('TaxJar settings saved.'));
}
