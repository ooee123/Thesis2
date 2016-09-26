package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class Parameter {
    private Type type;
    private String formalParameterName;
}
