--sys_msg_def
sys_msg =
{

    --login
    [1] = "sg_login_passward_error",
    [2] = "sg_login_no_register",
    [3] = "sg_login_version_error",
    [4] = "sg_login_repeat_login",
    [5] = "sg_login_status_err",


    --select_role
    [6] = "sg_select_role_roleid_err",
    [7] = "sg_select_role_already_del",


    --del_role
    [8] = "sg_del_role_roleid_err",
    [9] = "sg_del_role_already_del",


    --recover_role
    [10] = "sg_recover_role_roleid_err",
    [11] = "sg_recover_role_status_normal",
    [12] = "sg_recover_role_status_remove",
    [13] = "sg_recover_role_emoney_not_enough",
    [14] = "sg_recover_role_amount_exceeded",


    --create_role
    [15] = "sg_create_role_name_exist",
    [16] = "sg_create_role_not_login",
    [17] = "sg_create_role_amount_exceeded",
    [18] = "sg_create_role_type_error",


    --register
    [19] = "sg_register_account_exist",


    --assistance
    [20] = "sg_assistance_no_req_list",
    [21] = "sg_assistance_select_id_not_in_list",


    --reborn
    [22] = "sg_reborn_emoney_not_enough",


    --push_tower
    [23] = "sg_push_tower_enter_level_nomatch",
    [24] = "sg_push_tower_times_exceeded",
    [25] = "sg_push_tower_settle_gameinfo_not_exist",
    [26] = "sg_push_tower_settle_items_not_right",
    [27] = "sg_push_tower_settle_round_not_enouth",
    [28] = "sg_push_tower_settle_cost_round_illegel",
    [29] = "sg_push_tower_settle_gameid_not_match",


    --game
    [30] = "sg_game_settle_error",
    [31] = "sg_game_settle_item_exceeded",
    [32] = "sg_game_settle_gold_exceeded",
    [33] = "sg_game_not_enough_power",
    [34] = "sg_game_not_enough_summon_stone",
    [35] = "sg_game_copy_lock",
    [36] = "sg_game_emoney_not_enough",
    [37] = "sg_game_level_not_enough",


    --challenge
    [38] = "sg_challenge_enemy_noexist",
    [39] = "sg_challenge_noreq_list",
    [40] = "sg_challenge_times_use_up",
    [41] = "sg_challenge_self",
    [42] = "sg_challenge_level_err",
    [43] = "sg_challenge_settle_not_info",
    [44] = "sg_challenge_settle_info_err",
    [45] = "sg_challenge_rank_award_in_cd",
    [46] = "sg_challenge_rank_award_not_in_rank",


    --equipment_takeoff
    [47] = "sg_equipment_takeoff_pack_full",
    [48] = "sg_equipment_takeoff_type_error",


    --equipment_puton
    [49] = "sg_equipment_puton_noexist",
    [50] = "sg_equipment_puton_levelerr",
    [51] = "sg_equipment_puton_roletypeerr",
    [52] = "sg_equipment_puton_notowner",
    [53] = "sg_equipment_puton_itemtypeerr",


    --equipment_streng
    [54] = "sg_equipment_streng_cannot_streng",
    [55] = "sg_equipment_streng_gold_not_enough",
    [56] = "sg_equipment_streng_item_not_enough",
    [57] = "sg_equipment_streng_streng_failed",


    --equipment_advance
    [58] = "sg_equipment_advance_can_not_advance",
    [59] = "sg_equipment_advance_gold_not_enough",
    [60] = "sg_equipment_advance_item_not_enough",
    [61] = "sg_equipment_advance_level_not_enough",


    --equipment_recast
    [62] = "sg_equipment_recast_can_not_recast",
    [63] = "sg_equipment_recast_gold_not_enough",
    [64] = "sg_equipment_recast_item_not_enough",


    --equipment_resolve
    [65] = "sg_equipment_resolve_disable",
    [66] = "sg_equipment_resolve_on_body",
    [67] = "sg_equipment_resolve_not_exist",


    --gem_mount
    [68] = "sg_gem_mount_typeexist",
    [69] = "sg_gem_mount_not_trough",


    --equipment_save_recast
    [70] = "sg_equipment_save_recast_iderr",


    --gem_compound
    [71] = "sg_gem_compound_not_related",
    [72] = "sg_gem_compound_not_enough_gold",
    [73] = "sg_gem_compound_not_protect",
    [74] = "sg_gem_compound_gem_not_enough",
    [75] = "sg_gem_compound_pack_full",


    --gem_unmounted
    [76] = "sg_gem_unmounted_equip_notexist",
    [77] = "sg_gem_unmounted_pack_full",
    [78] = "sg_gem_unmounted_emoney_not_enough",
    [79] = "sg_gem_unmounted_notexist",


    --friend_add
    [80] = "sg_friend_add_limit_exceeded",
    [81] = "sg_friend_add_aim_limit_exceeded",
    [82] = "sg_friend_add_self",
    [83] = "sg_friend_add_exist",
    [84] = "sg_friend_add_offline",


    --mall_buy
    [85] = "sg_mall_buy_times_exceeded",
    [86] = "sg_mall_buy_money_not_enough",
    [87] = "sg_mall_buy_not_on_sale",
    [88] = "sg_mall_buy_pack_exceeded",


    --point_mall_buy
    [89] = "sg_point_mall_buy_rank_not_enough",
    [90] = "sg_point_mall_buy_point_not_enough",
    [91] = "sg_point_mall_buy_not_on_sale",
    [92] = "sg_point_mall_buy_pack_exceeded",


    --power_hp
    [93] = "sg_power_hp_emoney_not_enough",
    [94] = "sg_power_hp_limit_exceeded",


    --extend_pack
    [95] = "sg_extend_pack_is_max",
    [96] = "sg_extend_pack_emoney_not_enough",


    --pack_sale
    [97] = "sg_pack_sale_not_exists",
    [98] = "sg_pack_sale_amount_error",


    --summon_stone
    [99] = "sg_summon_stone_already_award",
    [100] = "sg_summon_stone_emoney_not_enough_to_buy",
    [101] = "sg_summon_stone_buy_times_exceeded",


    --sculpture
    [102] = "sg_sculpture_upgrade_money_not_enough",
    [103] = "sg_sculpture_upgrade_is_max_lev",
    [104] = "sg_sculpture_takeoff_empty",
    [105] = "sg_sculpture_puton_noexist",
    [106] = "sg_sculpture_puton_role_type_not_match",
    [107] = "sg_sculpture_divine_money_not_enough",
    [108] = "sg_sculpture_takeoff_pack_full",
    [109] = "sg_sculpture_puton_skill_repeat",
    [110] = "sg_sculpture_frag_not_enough",
    [111] = "sg_sculpture_divine_pack_full",
    [112] = "sg_sculpture_convert_pack_full",
    [113] = "sg_sculpture_divine_level_up",
    [114] = "sg_sculpture_divine_level_down",
    [115] = "sg_sculpture_pos_has_puton",
    [116] = "sg_sculpture_sale_is_on_body",
    [117] = "sg_sculpture_sale_noexist",
    [118] = "sg_sculpture_upgrade_is_expsculp",
    [119] = "sg_sculpture_puton_is_expsculpture",


    --task
    [120] = "sg_task_not_exist",
    [121] = "sg_task_monster_amount_not_enough",
    [122] = "sg_task_not_pass",
    [123] = "sg_task_not_full_star_pass",
    [124] = "sg_task_not_kill_all_pass",
    [125] = "sg_task_has_finished",
    [126] = "sg_task_player_level_not_enough",
    [127] = "sg_task_item_amount_not_enough",
    [128] = "sg_task_sculpture_upgrade_amount_not_enough",
    [129] = "sg_task_advance_equipment_amount_not_enough",
    [130] = "sg_task_equipment_resolve_amount_not_enough",
    [131] = "sg_task_sculpture_divine_amount_not_enough",


    --daily_award
    [132] = "sg_daily_award_get_already",
    [133] = "sg_daily_award_cannot_get",


    --broadcast
    [134] = "sg_broadcast_divine_sculpture_lev5",
    [135] = "sg_broadcast_convert_sculpture_lev6",
    [136] = "sg_broadcast_buy_sculpture_lev6",


    --clean_up_copy
    [137] = "sg_clean_up_copy_not_card",
    [138] = "sg_clean_up_copy_not_max_score",
    [139] = "sg_clean_up_copy_not_base_copy",
    [140] = "sg_clean_up_copy_ph_not_enough",


    --train_match
    [141] = "sg_train_match_buy_times_not_enough_emoney",
    [142] = "sg_train_match_refresh_not_enough_emoney",
    [143] = "sg_train_match_against_enemy_not_exist",
    [144] = "sg_train_match_against_leverr",
    [145] = "sg_train_match_times_exceeded",
    [146] = "sg_train_match_has_train",
    [147] = "sg_train_match_award_times_not_enough",
    [148] = "sg_train_match_award_has_get",
    [149] = "sg_train_match_settle_not_info",
    [150] = "sg_train_match_settle_info_error",


    --use_props
    [151] = "sg_use_props_not_props",
    [152] = "sg_use_props_not_exists",


    --benison
    [153] = "sg_benison_refresh_not_enough_gold",
    [154] = "sg_benison_bless_id_not_exist",
    [155] = "sg_benison_bless_emoney_not_enough",
    [156] = "sg_benison_bless_has_active",


    --activeness
    [157] = "sg_activeness_reward_point_not_enough",
    [158] = "sg_activeness_reward_has_gotten",


    --reconnect
    [159] = "sg_reconnect_token_err",


    --ladder_match
    [160] = "sg_ladder_match_award_gotten",
    [161] = "sg_ladder_match_award_disable",
    [162] = "sg_ladder_match_times_exceeded",
    [163] = "sg_ladder_match_emoney_not_enough",
    [164] = "sg_ladder_match_settle_error",


    --equipment_exchange
    [165] = "sg_equipment_exchange_disable",
    [166] = "sg_equipment_exchange_gold_err",
    [167] = "sg_equipment_exchange_meterial",


    --online_award
    [168] = "sg_online_award_lev_err",
    [169] = "sg_online_award_time_err",
    [170] = "sg_online_award_has_gotten",


    --exchange
    [171] = "sg_exchange_item_not_enough"
}