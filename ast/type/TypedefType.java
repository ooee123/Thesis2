package ast.type;

import lombok.Data;

/**
 * Created by ooee on 9/24/16.
 */
@Data
public abstract class TypedefType implements ActualType {
    public abstract String expandedStructUnion();

    public abstract void setTypedefName(String name);
}
