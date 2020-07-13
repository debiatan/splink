////////////////////////////////
// NOTE(allen): User Operations

internal void
OP_ViewPageFromDefinition(E_Definition *definition){
    E_View *switch_view = APP_GetPageView(definition);
    if (switch_view == 0){
        switch_view = E_NewView();
        E_InitPageView(switch_view, &vars->font, definition);
    }
    APP_SignalViewChange(switch_view);
}

internal void
OP_LookAtDefinition(E_Definition *definition){
    if (vars->active_view != 0){
        E_ViewLookAtDefinition(vars->active_view, definition);
        E_Tile *tile = E_GetTileForDefinition(vars->active_view->first_tile, definition);
        APP_SignalSnapToTile(tile);
    }
}
