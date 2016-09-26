package ast.type;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class PrimitiveType implements Type {
    private String typeName;
}
