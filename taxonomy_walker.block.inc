<?php

function taxonomy_walker_block_info() {
  $blocks = array();
  $block_count = taxonomy_walker_block_count();
  for($i = 0; $i < $block_count; $i++) {
    $blocks['taxonomy_walker_' . $i] = array(
      'info' => 'Taxonomy Walker - ' . $i,
      'cache' => DRUPAL_NO_CACHE,
    );    
  }
  return $blocks;
}

function taxonomy_walker_block_view($delta = '') {
  $block = array();

  switch($delta) {
    default:
      if($term_id = _taxonomy_walker_term_id_from_path()) {
        $term_id = $term_id;
      } else {
        $term_id = taxonomy_walker_block_tid($delta);
      }

      $block['subject'] = NULL;
      $block['content'] = array();
      $block['content']['#prefix'] = '<div id="taxonomy-walker-block">';
      $block['content']['#suffix'] = '</div>';
      $block['content']['content'] = drupal_get_form('taxonomy_walker_form', array('form_id' => $delta, 'term_id' => $term_id));
  }

  return $block;
}

function taxonomy_walker_block_configure($delta) {
  $form = array();

  $form['term_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Starting term ID',
    '#default_value' => taxonomy_walker_block_tid($delta),
  );
  return $form;
}


function taxonomy_walker_block_save($delta, $edit) {
  taxonomy_walker_block_tid($delta, $edit['term_id']); 
}
