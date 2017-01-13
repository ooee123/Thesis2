package ast.type;

import ast.BaseElement;

/**
 * Created by ooee on 9/24/16.
 */
public interface Type extends BaseElement {
    @Override
    default String toCode(boolean showOriginalLine) {
        return toCode();
    }

    String toCode();
}
