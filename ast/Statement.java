package ast;

import java.util.Set;

/**
 * Created by ooee on 9/24/16.
 */
public interface Statement extends BlockItem {
    Set<String> getDependantVariables();

    Set<String> getChangedVariables();
}
