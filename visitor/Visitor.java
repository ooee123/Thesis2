package visitor;

import ast.Statement;

import java.util.Collection;

/**
 * Created by ooee on 10/22/16.
 */
public interface Visitor<V> {
    void visit(V v);
}
