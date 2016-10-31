package ast;

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
        return type.toString() + " " + formalParameterName;
    }
}
