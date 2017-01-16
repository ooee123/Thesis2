package ast.type;

import ast.AbstractParameterList;
import lombok.Value;

/**
 * Created by ooee on 12/12/16.
 */
@Value
public class FunctionPointer implements ActualType {

    private Type returnType;
    private AbstractParameterList abstractParameterList;

    @Override
    public String toCode() {
        return null;
    }

    public String toCode(String identifier) {
        return returnType.toCode() + " (" + identifier + ")" + abstractParameterList.toCode();
    }

    @Override
    public String toExpandedCode() {
        return toCode();
    }
}
