package ast.declaration;

import ast.BlockItem;

/**
 * Created by ooee on 11/3/16.
 */
public interface Declaration extends BlockItem {
    @Override
    default boolean hasJump() {
        return false;
    }
}
