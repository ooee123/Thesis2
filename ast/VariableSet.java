package ast;

import java.util.Set;

/**
 * Created by ooee on 11/7/16.
 */
public class VariableSet {
    private Set<String> variables;

    public boolean containsKey(String variable) {
        if (variable.contains(".")) {
            String[] split = variable.split(".");
            return variables.contains(split[0]);
        }
        return variables.contains(variable);
    }
}
