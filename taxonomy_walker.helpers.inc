<?php

function _taxonomy_walker_field_value($field, $key = 'safe_value') {
  if(is_array($field)) {
    $field_1 = array_shift($field);
    if(is_array($field_1)) {
      $field_2 = array_shift($field_1);
      if(is_array($field_2)) {
        if(isset($field_2[$key])) {
          $field_value = $field_2[$key];
          return $field_value;                  
        }
      }
    }
  } 

  return false;
}


function _taxonomy_walker_array_item($array, $i) {
  $count = count($array);
  $item = ($count > $i) ? $array[$i] : $array[$count - 1];
  return $item;
}

/* Specify a set of options, select one or more with a set of keys */
function _taxonomy_walker_include_options($keys, $options) {
  $result = array();
  foreach($keys as $i => $key) {
    $result[$key] = $options[$key];
  }
  return $result;
}

/* Load Questions and Answers for a given term */
function taxonomy_walker_question_load($tid) {
  $term = taxonomy_term_load($tid);
  $question = $term->name;  
  if(isset($term->field_term_as_question)) {
    $field_value = _taxonomy_walker_field_value($term->field_term_as_question);
    if($field_value) {
      $question = $field_value;
    }
  } 
  return $question;
}

function taxonomy_walker_answers_load($tid) {
  $children = taxonomy_get_children($tid);
  $answers = array();
  foreach($children as $tid => $term) {
    $answer = $term->name;  
    if(isset($term->field_term_as_answer)) {
      $field_value = _taxonomy_walker_field_value($term->field_term_as_answer);
      if($field_value) {
        $answer = $field_value;
      }
    }
    $answers[$tid] = $answer;
  }
  return $answers;
}

function taxonomy_walker_answer_as_image($tid) {
  $term = taxonomy_term_load($tid);
  $src = 'http://emdeon.csclientsite.com/sites/all/modules/_custom/frontpage_context/images/physicians.png';
  if(isset($term->field_term_as_image)) {
    $field_value = _taxonomy_walker_field_value($term->field_term_as_image, 'uri');
    if($field_value) {
      $src = $field_value;
    }
  }  
  return $src;
}

function taxonomy_walker_term_content($term_id) {
  $content = array();
  $nodes = taxonomy_select_nodes($term_id);
  foreach($nodes as $index => $id) {
    $content[$index] = node_view(node_load($id), 'teaser');
  }
  return $content;
}


/* Helpers */
function _taxonomy_walker_find_in_array($array, $key) {
  if(isset($array[$key])) {
    return $array[$key];
  } elseif( is_array($array) ) {
    return _taxonomy_walker_find_in_array(array_shift($array), $key);
  } else {
    return false;
  }
}

function _taxonomy_walker_term_ids($answers, $end_point = false) {
  $term_ids = array();
  $previous_pointer = false;
  foreach($answers as $key => $answer) {
    $current_pointer = _taxonomy_walker_find_in_array($answer, 'select');

    if(!$previous_pointer) {
      $term_ids[] = $key;
    } elseif($key != $previous_pointer) {
      break;
    }

    if(!$current_pointer) {
      break;
    }

    if($end_point && $end_point == $key) {
      break;
    }

    $term_ids[] = $current_pointer;
    $previous_pointer = $current_pointer;
  }
  return $term_ids;
}

function _taxonomy_walker_update_term_ids($term_ids, $current_term_id, $parent_term_id = false) {
  if($parent_term_id) {
    /* Find the parent in the list */
    $position = array_search($parent_term_id, $term_ids);
    /* Cut everything out after it */
    $term_ids = array_slice($term_ids, 0, $position + 1);
  } else {
    $term_ids = array();
  }
  /* Now add new selection to the list */
  $term_ids[] = $current_term_id;

  return $term_ids;
}

function _taxonomy_walker_triggering_element_parse($form_state) {
  $element = $form_state['triggering_element'];
  $parents = $element['#parents'];
  $array = _taxonomy_walker_triggering_element_parse_parents($parents);

  if($element['#type'] == 'select' || $element['#type'] == 'radios') {
    $array['current_term_id'] = $form_state['values']['answer'][$array['parent_term_id']]['select'];
  } 

  return $array;
}

function _taxonomy_walker_triggering_element_parse_parents($parents) {
  $current_term_id = false;
  $parent_term_id = false;

  foreach ($parents as $index => $value) {
    if(is_numeric($value)) {
      if($parent_term_id === false) {
        $parent_term_id = $value;
      } elseif($current_term_id === false) {
        $current_term_id = $value;
        break;
      }
    }
  }

  return array(
    'current_term_id' => $current_term_id,
    'parent_term_id' => $parent_term_id,
  );
}

function _taxonomy_walker_term_id_from_path() {
  if(arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2))) {
    return arg(2);
  } else {
    return false;
  }
}
