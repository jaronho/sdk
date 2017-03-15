sex_type = 
{
    ["boy"]= 1,
    ["girl"] = 2
}
game_type = 
{
    ["common"]= 1,
    ["push_tower"]= 2,
    ["game_challenge"]= 3,
    ["copy_clean_up"] = 4
}
user_operation = 
{
    ["opt_open"]= 1,
    ["opt_atk_monster"]= 2,
    ["opt_pickup_item"]= 3,
    ["opt_use_skill"]= 4,
    ["opt_use_item"]= 5,
    ["opt_pass"] = 6
}
game_result = 
{
    ["game_win"]= 1,
    ["game_lost"]= 2,
    ["game_error"] = 3
}
register_result = 
{
    ["register_success"]= 1,
    ["register_failed"] = 2
}
login_result = 
{
    ["login_success"]= 1,
    ["login_noregister"]= 2,
    ["login_passworderror"]= 3,
    ["login_norole"]= 4,
    ["login_versionerror"]= 5,
    ["login_status_err"] = 6
}
reconnect_result = 
{
    ["reconnect_success"]= 1,
    ["reconnect_noregister"]= 2,
    ["reconnect_passworderror"]= 3,
    ["reconnect_versionerror"] = 4
}
create_role_result = 
{
    ["create_role_success"]= 1,
    ["create_role_failed"]= 2,
    ["create_role_nologin"]= 3,
    ["create_role_typeerror"]= 4,
    ["create_role_nameexisted"] = 5
}
enter_game_result = 
{
    ["enter_game_success"]= 1,
    ["enter_game_failed"]= 2,
    ["enter_game_unlogin"] = 3
}
common_result = 
{
    ["common_success"]= 1,
    ["common_failed"]= 2,
    ["common_error"] = 3
}
equipment_type = 
{
    ["weapon"]= 1,
    ["armor"]= 2,
    ["necklace"]= 3,
    ["ring"]= 4,
    ["jewelry"]= 5,
    ["medal"] = 6
}
item_type = 
{
    ["equipment"]= 1,
    ["sculpture"]= 2,
    ["gem"]= 3,
    ["varia"]= 4,
    ["props"]= 5,
    ["material"]= 6,
    ["rand_props"] = 7
}
quality_type = 
{
    ["white"]= 1,
    ["green"]= 2,
    ["blue"]= 3,
    ["purple"]= 4,
    ["orange"] = 5
}
data_type = 
{
    ["init"]= 1,
    ["append"]= 2,
    ["delete"]= 3,
    ["modify"] = 4
}
role_status = 
{
    ["online"]= 1,
    ["offline"] = 2
}
answer_type = 
{
    ["agree"]= 1,
    ["defuse"] = 2
}
map_settle_result = 
{
    ["map_settle_next_map"]= 1,
    ["map_settle_finish"]= 2,
    ["map_settle_died"]= 3,
    ["map_settle_error"] = 4
}
relation = 
{
    ["friend"]= 1,
    ["other"] = 2
}
price_type = 
{
    ["price_type_gold"]= 1,
    ["price_type_emoney"] = 2
}
mall_item_state = 
{
    ["on_sale"]= 1,
    ["not_on_sale"] = 2
}
award_type = 
{
    ["daily_award"]= 1,
    ["cumulative_award3"]= 2,
    ["cumulative_award7"]= 3,
    ["cumulative_award15"] = 4
}
ladder_award_type = 
{
    ["ladder_daily_award"]= 1,
    ["ladder_weekiy_award"] = 2
}
bind_type = 
{
    ["bind_never"]= 1,
    ["bind_default"]= 2,
    ["bind_puton"] = 3
}
bind_status = 
{
    ["bind"]= 1,
    ["not_bind"] = 2
}
lottery_type = 
{
    ["lottery_item"]= 1,
    ["lottery_gold"]= 2,
    ["lottery_exp"] = 3
}
rank_type = 
{
    ["battle_power_rank"]= 1,
    ["role_level_rank"] = 2
}
train_award_type = 
{
    ["blue_award"]= 1,
    ["purple_award"]= 2,
    ["orange_award"]= 3,
    ["red_award"] = 4
}
sculpture_item_type = 
{
    ["item_sculpture"]= 1,
    ["item_talent"]= 2,
    ["item_frag"]= 3,
    ["item_expsculp"] = 4
}
gm_opt_type = 
{
    ["gm_opt_exp"]= 1,
    ["gm_opt_gold"]= 2,
    ["gm_opt_emoney"]= 3,
    ["gm_opt_summon_stone"]= 4,
    ["gm_opt_point"]= 5,
    ["gm_opt_honour"]= 6,
    ["gm_opt_item"]= 7,
    ["gm_opt_sculpture"]= 8,
    ["gm_opt_frag"]= 9,
    ["gm_opt_recharge"] = 10
}
first_charge_status = 
{
    ["not_charge"]= 1,
    ["charget_not_rewarded"]= 2,
    ["rewarded"] = 3
}

function get_proto_version()

    return 61;
end
