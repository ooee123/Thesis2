package ast.type;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Data
public class TypedefType implements Type {
    private String typeName;

    public TypedefType(String typeName) {
        this.typeName = typeName;
    }
}
