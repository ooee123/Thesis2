package ast;

import java.util.Set;

/**
 * Created by ooee on 10/9/16.
 */
public interface Assigning extends Expression {
    Set<String> getRightVariables();
}
