<?php

function taxonomy_walker_user_selected_terms() {
  return $_COOKIE['Drupal_visitor_taxonomy_walker_tids'];
}

