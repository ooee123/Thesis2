package ast.type;

import lombok.Value;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class PointerType implements Type {
    private int pointerLevel;
    private Type type;
}
