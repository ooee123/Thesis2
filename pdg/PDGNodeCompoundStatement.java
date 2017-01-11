package pdg;

import ast.*;
import ast.statement.Statement;
import ast.statement.impl.CompoundStatement;
import com.google.common.collect.Lists;
import lombok.EqualsAndHashCode;
import lombok.Value;

import java.util.Collection;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeCompoundStatement extends PDGNode<CompoundStatement> {
    private Collection<PDGNode<? extends BlockItem>> body;
    private VariableMap lastAssigned;

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode<? extends BlockItem>> body, VariableMap lastAssigned) {
        this(compoundStatement, body, lastAssigned, false);
    }

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode<? extends BlockItem>> body, VariableMap lastAssigned, boolean required) {
        super(compoundStatement, required);
        this.body = body;
        this.lastAssigned = lastAssigned;
    }

    @Override
    public CompoundStatement sort(PDGSorter sorter) {
        Statement sort = sorter.sort(body);
        if (sort instanceof CompoundStatement) {
            return ((CompoundStatement) sort);
        } else {
            return new CompoundStatement(Lists.newArrayList(sort));
        }
    }
}
