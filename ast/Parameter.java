package ast;

import ast.type.PointerType;
import ast.type.Type;
import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class Parameter implements BaseElement {
    @NonNull private Type type;
    @NonNull private PrimaryExpressionIdentifier formalParameterName;

    @Override
    public String toCode() {
        if (type instanceof PointerType) {
            return type.toCode() + formalParameterName.toCode();
        } else {
            return type.toCode() + " " + formalParameterName.toCode();
        }
    }
}
