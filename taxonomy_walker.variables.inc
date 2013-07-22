<?php

function taxonomy_walker_default_vocabulary($set = false) {
  if(!$set) {
    return variable_get('taxonomy_walker_default_vocabulary', 11); 
  } else {
    variable_set('taxonomy_walker_default_vocabulary', $set);
  }
}

function taxonomy_walker_default_term($set = false) {
  if(!$set) {
    return variable_get('taxonomy_walker_default_term', 15); 
  } else {
    variable_set('taxonomy_walker_default_term', $set);
  }
}

function taxonomy_walker_path($set = false) {
  // if(!$set) {
  //   return variable_get('taxonomy_walker_path', 'taxonomy-walker'); 
  // } else {
  //   variable_set('taxonomy_walker_path', $set);
  // }
  return 'taxonomy-walker';
}

function taxonomy_walker_block_count($set = false) {
  if(!$set) {
    return variable_get('taxonomy_walker_block_count', 1); 
  } else {
    variable_set('taxonomy_walker_block_count', $set);
  }
}

function taxonomy_walker_block_tid($delta, $set = false) {
  if(!$set) {
    return variable_get('taxonomy_walker_' . $delta . '_tid', taxonomy_walker_default_term()); 
  } else {
    variable_set('taxonomy_walker_' . $delta . '_tid', $set);
  }
}
